/* radare - LGPL - Copyright 2009-2013 - pancake, nibble */

typedef void (*HashHandler)(const ut8 *block, int len);

static void handle_md4 (const ut8 *block, int len);
static void handle_md5 (const ut8 *block, int len);
static void handle_sha1 (const ut8 *block, int len);
static void handle_sha256 (const ut8 *block, int len);
static void handle_sha512 (const ut8 *block, int len);
static void handle_adler32 (const ut8 *block, int len);
static void handle_crc32 (const ut8 *block, int len);
static void handle_crc16 (const ut8 *block, int len);
static void handle_xor (const ut8 *block, int len);
static void handle_entropy (const ut8 *block, int len);
static void handle_hamdist (const ut8 *block, int len);
static void handle_pcprint (const ut8 *input, int len);

typedef struct {
	const char *name;
	HashHandler handler;
} RHashHashHandlers;

static RHashHashHandlers HASH_HANDLERS[] = {
	{"md4", handle_md4},
	{"md5", handle_md5},
	{"sha1", handle_sha1},
	{"sha256", handle_sha256},
	{"sha512", handle_sha512},
	{"adler32", handle_adler32},
	{"crc32", handle_crc32},
	{"crc16", handle_crc16},
	{"xor", handle_xor},
	{"entropy", handle_entropy},
	{"hamdist", handle_hamdist},
	{"pcprint", handle_pcprint},
	{NULL, NULL},
};

static void handle_md4 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (R_TRUE, R_HASH_MD4);
	const ut8 *c = r_hash_do_md4 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_MD4; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_md5 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (R_TRUE, R_HASH_MD5);
	const ut8 *c = r_hash_do_md5 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_MD5; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha1 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (R_TRUE, R_HASH_SHA1);
	const ut8 *c = r_hash_do_sha1 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_SHA1; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha256 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (R_TRUE, R_HASH_SHA256);
	const ut8 *c = r_hash_do_sha256 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_SHA256; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha512 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (R_TRUE, R_HASH_SHA512);
	const ut8 *c = r_hash_do_sha512 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_SHA512; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_adler32 (const ut8 *block, int len) {
	ut32 hn = r_hash_adler32 (block, len);
	ut8 *b = (ut8*)&hn;
	r_cons_printf ("%02x%02x%02x%02x\n", b[0], b[1], b[2], b[3]);
}

static void handle_crc32 (const ut8 *block, int len) {
	r_cons_printf ("%04x\n", r_hash_crc32 (block, len));
}

static void handle_crc16 (const ut8 *block, int len) {
	r_cons_printf ("%04x\n", r_hash_crc16 (0, block, len));
}

static void handle_xor (const ut8 *block, int len) {
	r_cons_printf ("%02x\n", r_hash_xor (block, len));
}

static void handle_entropy (const ut8 *block, int len) {
	r_cons_printf ("%02x\n", r_hash_entropy (block, len));
}

static void handle_hamdist (const ut8 *block, int len) {
	r_cons_printf ("%02x\n", r_hash_hamdist (block, len));
}

static void handle_pcprint (const ut8 *block, int len) {
	r_cons_printf ("%d\n", r_hash_pcprint (block, len));
	//r_cons_printf ("%02x\n", r_hash_pcprint (block, len));
}

static void algolist(int mode) {
	const char *name;
	ut64 bits;
	int i;
	for (i=0; ; i++) {
		bits = 1<<i;
		name = r_hash_name (bits);
		if (!name||!*name) break;
		if (mode) {
			eprintf ("%s\n", name);
		} else {
			eprintf (" #%s", name);
			if (!((i+1)%10)) r_cons_newline ();
		}
	}
	if (!mode) eprintf ("\n");
}

static int cmd_hash(void *data, const char *input) {
	char *p, algo[32];
	RCore *core = (RCore *)data;
	ut32 osize = 0, len = core->blocksize;
	const char *ptr;
	int pos = 0, handled_cmd = R_FALSE;

	if (!*input) return 0;
	else if (input[0]==' ') return 0;
	if (input[0]=='#' && !input[1]) {
		algolist (1);
		return R_TRUE;
	}
	if (input[0]=='!') {
		const char *lang = input+1;
		if (*lang=='/') {
			const char *ptr = lang+1;
			while (*lang) {
				if (*lang=='/')
					ptr = lang+1;
				lang++;
			}
			RLangPlugin *p = r_lang_get_by_extension (core->lang, ptr);
			if (p && p->name) lang = p->name;
		}
		if (*lang==' ') {
			RLangPlugin *p = r_lang_get_by_extension (core->lang, input+2);
			if (p && p->name) lang = p->name;
		} else if (input[1]=='?' || input[1]=='*' || input[1]=='\0') {
			r_lang_list (core->lang);
			return R_TRUE;
		}
		p = strchr (input, ' ');
		if (p) *p=0;
		// TODO: set argv here
		if (r_lang_use (core->lang, lang)) {
			r_lang_setup (core->lang);
			if (p) r_lang_run_file (core->lang, p+1);
			else r_lang_prompt (core->lang);
		} else
		if (!p || *p)
			eprintf ("Invalid hashbang. See '#!' for help.\n");
		return R_TRUE;
	}

	ptr = strchr (input, ' ');
	sscanf (input, "%31s", algo);
	if (ptr && *(ptr+1) && r_num_is_valid_input (core->num, ptr+1)) {
		int nlen = r_num_math (core->num, ptr+1);
		if (nlen>0) len = nlen;
		osize = core->blocksize;
		if (nlen>core->blocksize) {
			r_core_block_size (core, nlen);
		}
	} else if (!ptr || !*(ptr+1)) osize = len;
	/* TODO: Simplify this spaguetti monster */

	while (osize > 0 && HASH_HANDLERS[pos].name != NULL) {
		if (!r_str_ccmp (input, HASH_HANDLERS[pos].name, ' ')) {
			HASH_HANDLERS[pos].handler (core->block, len);
			handled_cmd = R_TRUE;
			break;
		}
		pos++;
	}

	if (!osize) {
		eprintf ( "Error: provided size must be size > 0\n" );
	}

	if (input[0]=='?' || handled_cmd == R_FALSE) {
		eprintf ("Usage: #algo <size> @ addr\n"
		" # this is a comment   note the space after the sharp sign\n"
		" ##                    List hash/checksum algorithms.\n"
		" #sha256 10K @ 33      calculate sha256 of 10K at 33\n"
		"Hashes:\n");
		algolist (0);
		eprintf (
		"Usage #!interpreter [<args>] [<file] [<<eof]\n"
		" #!                    list all available interpreters\n"
		" #!python              run python commandline\n"
		" #!python foo.py       run foo.py python script (same as '. foo.py')\n"
		//" #!python <<EOF        get python code until 'EOF' mark\n"
		" #!python arg0 a1 <<q  set arg0 and arg1 and read until 'q'\n");
	}
	if (osize)
		r_core_block_size (core, osize);
	return 0;
}
