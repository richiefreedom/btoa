/**
 *  Binary file to assembly code converter.
 *  Author: Sergei V. Rogachev, RogachevSergei{at}gmail{dot}com.
 *  License: GNU GPL v3.
 *
 *  A simple but very useful tool helped me many times. Not all assembly
 *  language translators support a special command for inclusion of binary
 *  files to your code. This utility helps to do such thing the simplest
 *  possible way - by usge of basic data definition mnemonic which can be
 *  found in any assembler.
 *
 *  For definition of a byte we often use something like "db" in intel-like
 *  assemblers and ".byte" in AT&T ones. So, it is possible to define any data
 *  as an array of bytes, no matter what you want to inject to your final
 *  binary - some image, text or other kind of BLOB. It is necessary also to
 *  know the size of the included data.
 *
 *  btoa defines two symbols. The first one is a label to acces the first byte
 *  in the data array. The symbol name is the name of your input file + a
 *  postfix "_file". Of course, symbols not supported in assembly language are
 *  replaced simply by '_'. The second symbol is a long value containing a size
 *  of the data. Its name looks similar to the data label but with a little
 *  difference - a postfix "_size".
 *
 *  An example:
 *
 *  If we have executed btoa on the file named login-screen.bmp we will see two
 *  symbols in the output: login_screen_bmp_file, login_screen_bmp_file_size.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILENAME_LENGTH   256
#define DEF_OUT_FILENAME  "out.asm"

#define PROG_FILENAME_IDX 0
#define ARCH_NAME_IDX     1
#define IN_FILENAME_IDX   2
#define OUT_FILENAME_IDX  3

#define ELEMS_PER_LINE    8

static char *progname;
static char **args;

struct lang_spec {
	char *name;
	char *def;
	char *diff;
	char *glob_pre;
	char *glob_post;
};

struct lang_spec lang_specs[] = {
	{"nasm", "db", "dd $-", "[GLOBAL ", "]"},
	{"fasm", "db", "dd $-", "global ", ""},
	{"as", ".byte", ".long .-", ".globl ", ""},
	{NULL}
};

#define lang_specs_foreach(lang) \
	for (lang = lang_specs; lang->name; lang++)

/**
 * lookup_lang_spec() - find a specification of concrete assembly language.
 *
 * @name: the string with a name of the language.
 *
 * Retuns a pointer to the language specification or NULL.
 */
static struct lang_spec *lookup_lang_spec(const char *name)
{
	struct lang_spec *lang;

	lang_specs_foreach(lang) {
		if (0 == strcmp(lang->name, name))
			return lang;
	}

	return NULL;
}

static void print_langs(void)
{
	struct lang_spec *lang;

	lang_specs_foreach(lang) {
		fprintf(stderr, " %s", lang->name);
	}

	fprintf(stderr, "\n");
}

static void panic(char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);
}

static void usage(void)
{
	fprintf(stderr, "Format: %s <lang> <binary file name> "
			"[<assembly file name>]\n", progname);
	fprintf(stderr, "<lang> can be one of:");
	print_langs();
	fprintf(stderr, "\n");
}

/**
 * str_repl_char() - replace every occurance of some character in a string.
 *
 * @str: the string to be scanned through
 * @old: the replaceable symbol
 * @new: the replacement
 *
 * Returns a number of replaced symbols.
 */
static int str_repl_char(char *str, int old, int new)
{
	int ret = 0;

	while (*str) {
		if (*str == old) {
			*str = new;
			ret++;
		}
		str++;
	}

	return ret;
}

/**
 * convert() - convert data from input file to the code placed in output file
 * using a selected assembly language syntax and export a count of converted
 * bytes.
 *
 * @in:  the input file descriptor pointer
 * @out: the output file descriptor pointer
 * @lang: the language specification pointer
 * @count: the pointer to counter
 *
 * Returns 0 if OK, -1 if input error, -2 if output error.
 */
static int convert(FILE *in, FILE *out, struct lang_spec *lang, size_t *count)
{
	unsigned char byte;
	int ret;

	*count = 0;

	ret = fprintf(out, "%s%s_file%s\n", lang->glob_pre,
			args[IN_FILENAME_IDX], lang->glob_post);
	if (ret < 0)
		goto out_err;

	ret = fprintf(out, "%s_file:\n", args[IN_FILENAME_IDX]);
	if (ret < 0)
		goto out_err;

	for (;;) {
		size_t rwcount;

		rwcount = fread(&byte, 1, 1, in);
		if (!rwcount) {
			if (feof(in))
				break;
			if (ferror(in))
				goto in_err;
		}

		if (*count && (*count % ELEMS_PER_LINE)) {
			ret = fprintf(out, ",\t");
		} else {
			ret = fprintf(out, "\n%s\t", lang->def);
		}
		if (ret < 0)
			goto out_err;

		ret = fprintf(out, "0x%x", byte);
		if (ret < 0)
			goto out_err;

		(*count)++;
	}

	ret = fprintf(out, "\n\n%s%s_file_size%s\n", lang->glob_pre,
			args[IN_FILENAME_IDX], lang->glob_post);
	if (ret < 0)
		goto out_err;

	ret = fprintf(out, "%s_file_size: %s%s_file\n", args[IN_FILENAME_IDX],
			lang->diff, args[IN_FILENAME_IDX]);
	if (ret < 0)
		goto out_err;

	return 0;
in_err:
	return -1;
out_err:
	return -2;
}

static void cleanup(FILE *in, FILE *out)
{
	if (stdout != out)
		fclose(out);
	if (in)
		fclose(in);
}

int main(int argc, char **argv)
{
	FILE *in  = NULL;
	FILE *out = stdout;

	struct lang_spec *lang;
	size_t count;
	int    ret;

	progname = argv[PROG_FILENAME_IDX];
	args = argv;

	fprintf(stderr, "Binary file to assembly language converter.\n\n");

	if (argc < 3 || argc > 4) {
		usage();
		panic("At least two parameters are necessary.");
	}

	lang = lookup_lang_spec(argv[ARCH_NAME_IDX]);
	if (!lang)
		panic("Non-supported assembly syntax.");

	in = fopen(argv[IN_FILENAME_IDX], "r");
	if (!in)
		panic("Unable to open the input file.");

	if (4 == argc) {
		out = fopen(argv[OUT_FILENAME_IDX], "w");
		if (!out) {
			cleanup(in, out);
			panic("Unable to create a new file.");
		}
	}

	str_repl_char(argv[IN_FILENAME_IDX], '.', '_');
	str_repl_char(argv[IN_FILENAME_IDX], '-', '_');

	ret = convert(in, out, lang, &count);
	switch (ret) {
	case -1:
		cleanup(in, out);
		panic("Unable to read the input file.");
	case -2:
		cleanup(in, out);
		panic("Unable to write the output file. "
				"WARNING: Output data is inconsistent!");
	}

	fprintf(stderr, "%lu bytes have been converted.\n", count);

	cleanup(in, out);

	return 0;
}
