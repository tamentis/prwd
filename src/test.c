/*
 * Copyright (c) 2009-2014 Bertrand Janin <b@janin.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <stdarg.h>

#include "main.c"

#define RUN_TEST(f)				\
	printf(#f "... ");			\
	fflush(stdout);				\
	if (f()) {				\
		printf("PASS\n");		\
		passed++;			\
	} else {				\
		printf("FAIL\n");		\
		if (strlen(details)) {		\
			puts(details);		\
			details[0] = '\0';	\
		}				\
		failed++;			\
	};					\
	tested++;				\

#define ALIAS_ADD(a, b)				\
	alias_add(a, b, &aaerrstr);		\
	if (aaerrstr != NULL) {			\
		return (1);			\
	}					\


extern int alias_count;
const char *aaerrstr;
char details[256] = "";
char errstr[256] = "";
char test_hostname_value[MAXHOSTNAMELEN];
int test_hostname_return = 0;
int tested = 0;
int passed = 0;
int failed = 0;
int test_file_exists = 1;


/* Override errx during tests to capture the errors. */
void
errx(int eval, const char *fmt,...)
{
	(void)eval;
        va_list args;

	va_start(args, fmt);
	vsnprintf(errstr, sizeof(errstr), fmt, args);
	va_end(args);
}

/*
 * Override ensuring the hostname is predictable for the purpose of testing.
 */
int
get_full_hostname(char *buf, size_t size)
{
	strlcpy(buf, test_hostname_value, size);
	return (test_hostname_return);
}

/*
 * Override file_exists to ensure predictable returns.
 */
int
file_exists(char *filepath)
{
	(void)filepath;
	return (test_file_exists);
}

/*
 * Testing assertions
 */
static int
assert_string_equals(const char *a, const char *b)
{
	if (a == NULL && b != NULL)
		goto bad;
	if (a != NULL && b == NULL)
		goto bad;

	if (strcmp(a, b) == 0)
		return (1);

bad:
	snprintf(details, sizeof(details),
	    "strings do not match:\n"
	    "	a=%s\n"
	    "	b=%s\n", a, b);
	return (0);
}

static int
assert_wstring_equals(const wchar_t *a, const wchar_t *b)
{
	if (a == NULL && b != NULL)
		goto bad;
	if (a != NULL && b == NULL)
		goto bad;

	if (wcscmp(a, b) == 0)
		return (1);

bad:
	snprintf(details, sizeof(details),
	    "wide strings do not match:\n"
	    "	a=%ls\n"
	    "	b=%ls\n", a, b);
	return (0);
}

static int
assert_int_equals(int a, int b)
{
	if (a == b)
		return (1);

	snprintf(details, sizeof(details),
	    "ints do not match:\n"
	    "	a=%d\n"
	    "	b=%d\n", a, b);

	return (0);
}

static int
assert_null(const void *p)
{
	if (p == NULL)
		return (1);

	snprintf(details, sizeof(details), "pointer is not NULL");
	return (0);
}

/*
 * newgroupize tests
 */
void newsgroupize(wchar_t *);

static int
test_newsgroupize_null(void)
{
	newsgroupize(NULL);
	return (1);
}

static int
test_newsgroupize_empty(void)
{
	wchar_t s[] = L"";
	newsgroupize(s);
	return (*s == '\0');
}

static int
test_newsgroupize_one(void)
{
	wchar_t s[] = L"a";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"a"));
}

static int
test_newsgroupize_root(void)
{
	wchar_t s[] = L"/";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/"));
}

static int
test_newsgroupize_slash_one(void)
{
	wchar_t s[] = L"/a";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/a"));
}

static int
test_newsgroupize_tmp(void)
{
	wchar_t s[] = L"/tmp";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/tmp"));
}

static int
test_newsgroupize_home(void)
{
	wchar_t s[] = L"/home/tamentis";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/h/tamentis"));
}

static int
test_newsgroupize_shorthome(void)
{
	wchar_t s[] = L"~/projects/prwd";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"~/p/prwd"));
}

static int
test_newsgroupize_shorthome_one_level(void)
{
	wchar_t s[] = L"~/bin";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"~/bin"));
}

static int
test_newsgroupize_alreadyshort(void)
{
	wchar_t s[] = L"/a/b/c/d/e/f/g/h/i/j";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/a/b/c/d/e/f/g/h/i/j"));
}

static int
test_newsgroupize_trailingslash(void)
{
	wchar_t s[] = L"/usr/local/";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"/u/local/"));
}

static int
test_newsgroupize_alias(void)
{
	wchar_t s[] = L"$whatever/local/usr/share";
	newsgroupize(s);
	return (assert_wstring_equals(s, L"$whatever/l/u/share"));
}


/*
 * Quick Cut tests
 */
void	quickcut(wchar_t *, size_t);

static int
test_quickcut_null(void)
{
	quickcut(NULL, 0);
	return (1);
}

static int
test_quickcut_empty(void)
{
	wchar_t s[] = L"";
	quickcut(s, 0);
	return (assert_wstring_equals(s, L""));
}

static int
test_quickcut_one_to_one(void)
{
	wchar_t s[] = L"o";
	cfg_maxpwdlen = 1;
	quickcut(s, 1);
	return (assert_wstring_equals(s, L"o"));
}

static int
test_quickcut_one_to_two(void)
{
	wchar_t s[] = L"o";
	cfg_maxpwdlen = 2;
	quickcut(s, 1);
	return (assert_wstring_equals(s, L"o"));
}

static int
test_quickcut_thirty_to_ten(void)
{
	wchar_t s[] = L"qwertyuiopasdfghjklzxcvbnmqwer";
	cfg_maxpwdlen = 10;
	quickcut(s, 30);
	return (assert_wstring_equals(s, L"...bnmqwer"));
}

static int
test_quickcut_ten_to_thirty(void)
{
	wchar_t s[] = L"1234567890";
	cfg_maxpwdlen = 30;
	quickcut(s, 10);
	return (assert_wstring_equals(s, L"1234567890"));
}

static int
test_quickcut_ten_to_ten(void)
{
	wchar_t s[] = L"1234567890";
	cfg_maxpwdlen = 10;
	quickcut(s, 10);
	return (assert_wstring_equals(s, L"1234567890"));
}

/*
 * Clean cut tests
 */
void cleancut(wchar_t *s);

static int
test_cleancut_null(void)
{
	cleancut(NULL);
	return (1);
}

static int
test_cleancut_empty(void)
{
	wchar_t s[] = L"";
	cleancut(s);
	return (s[0] == L'\0');
}

static int
test_cleancut_root_to_ten(void)
{
	wchar_t s[] = L"/";
	cfg_maxpwdlen = 10;
	cleancut(s);
	return (assert_wstring_equals(s, L"/"));
}

static int
test_cleancut_root_to_one(void)
{
	wchar_t s[] = L"/";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"/"));
}

static int
test_cleancut_tmp_to_one(void)
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"/tmp"));
}

static int
test_cleancut_tmp_to_three(void)
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 3;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"/tmp"));
}

static int
test_cleancut_tmp_to_four(void)
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 4;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"/tmp"));
}

static int
test_cleancut_tmp_to_ten(void)
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"/tmp"));
}

static int
test_cleancut_uld_to_one(void)
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L".../doc"));
}

static int
test_cleancut_uld_to_five(void)
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 5;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L".../doc"));
}

static int
test_cleancut_uld_to_ten(void)
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L".../doc"));
}

static int
test_cleancut_uld_to_eleven(void)
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"_";
	cfg_maxpwdlen = 11;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	return (assert_wstring_equals(s, L"_/local/doc"));
}

/*
 * aliases tests
 */
void replace_aliases(wchar_t *);

static int
test_aliases_none(void)
{
	wchar_t pwd[] = L"/usr/local/doc";
	alias_purge_all();
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"/usr/local/doc"));
}

static int
test_aliases_home_alone(void)
{
	wchar_t pwd[] = L"/home/tamentis";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"~"));
}

static int
test_aliases_home_and_one(void)
{
	wchar_t pwd[] = L"/home/tamentis/x";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"~/x"));
}

static int
test_aliases_home_and_tree(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/projects/stuff";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"~/x/projects/stuff"));
}

static int
test_aliases_five_unmatching_aliases(void)
{
	wchar_t pwd[] = L"/home/tamentiz/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"a1", L"/the/first/path");
	ALIAS_ADD(L"b2", L"/path/second");
	ALIAS_ADD(L"c3", L"/troisieme/chemin");
	ALIAS_ADD(L"d4", L"drole/de/chemin/quatre");
	ALIAS_ADD(L"e5", L"/home/tamentïs");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"/home/tamentiz/x/projects"));
}

static int
test_aliases_duplicate_aliases(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"aa", L"/home/tamentis");
	ALIAS_ADD(L"aa", L"/home/tamentis");
	ALIAS_ADD(L"aa", L"/home/tamentis");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"aa/x/projects"));
}

static int
test_aliases_too_many(void)
{
	int i;
	alias_purge_all();
	for (i = 0; i < MAX_ALIASES * 2; i++) {
		ALIAS_ADD(L"aa", L"/home/tamentis");
	}

	alias_add(L"aa", L"/home/tamentis", &aaerrstr);
	if (aaerrstr == NULL) {
		snprintf(details, sizeof(details),
		    "alias_add should have returned an error");
		return (0);
	}

	return (assert_string_equals(aaerrstr, "too many aliases"));
}

static int
test_aliases_find_smallest(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/y/z/projects/prwd";
	alias_count = 0;
	ALIAS_ADD(L"bad1", L"/home/tamentis");
	ALIAS_ADD(L"bad2", L"/home");
	ALIAS_ADD(L"bad3", L"/home/tamentis/x");
	ALIAS_ADD(L"good", L"/home/tamentis/x/y/z");
	replace_aliases(pwd);
	return (assert_wstring_equals(pwd, L"good/projects/prwd"));
}

/*
 * config file parser test
 */
// void process_config_line(wchar_t *line, int linenum, const char **);

static int
test_config__process_config_line__set_no_var(void)
{
	wchar_t line[] = L"set";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_string_equals(aaerrstr, "set without variable name"));
}

static int
test_config__process_config_line__alias_no_name(void)
{
	wchar_t line[] = L"alias";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_string_equals(aaerrstr, "alias without name"));
}

static int
test_config__process_config_line__just_spaces(void)
{
	wchar_t line[] = L"        ";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_null(aaerrstr));
}

static int
test_config__process_config_line__comments(void)
{
	wchar_t line[] = L"# comments";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_null(aaerrstr));
}

static int
test_config__process_config_line__set_maxlength_250(void)
{
	wchar_t line[] = L"set maxlength 250";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_null(aaerrstr)
	    && assert_int_equals(cfg_maxpwdlen, 250));
}

static int
test_config__process_config_line__set_maxlength_bad(void)
{
	wchar_t line[] = L"set maxlength $F@#$";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_string_equals(aaerrstr, "invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_overflow(void)
{
	wchar_t line[] = L"set maxlength 5000";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_string_equals(aaerrstr, "invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_quoted(void)
{
	wchar_t line[] = L"set maxlength \"50\"";
	aaerrstr = "";
	process_config_line(line, &aaerrstr);
	return (assert_null(aaerrstr)
	    && assert_int_equals(cfg_maxpwdlen, 50));
}

/*
 * test the hostname feature
 */
void add_hostname(wchar_t *);

static int
test_hostname_full(void)
{
	wchar_t s[256] = L"anything";
	char h[] = "odin.tamentis.com";

	test_hostname_return = 0;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	return (assert_wstring_equals(s, L"odin:anything"));
}

static int
test_hostname_short(void)
{
	wchar_t s[256] = L"anything";
	char h[] = "odin";

	test_hostname_return = 0;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	return (assert_wstring_equals(s, L"odin:anything"));
}

static int
test_hostname_error_no_hostname(void)
{
	wchar_t s[256] = L"anything";
	char h[] = "odin";

	test_hostname_return = -1;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	return (assert_string_equals(errstr, "gethostname() failed"));
}

/*
 * test the expand alias tools
 */
static int
test_config_alias_expand_prefix(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what/the/fsck";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/anything/giving/the/fsck"));
}

static int
test_config_alias_expand_prefix_with_slash(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what/the/fsck/";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving/");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/anything/giving//the/fsck/"));
}

static int
test_config_alias_expand_prefix_single(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/anything/giving"));
}

/*
 * The configured alias does not match anything in the path and the output
 * should be identical to the input.
 */
static int
test_config_alias_expand_prefix_no_alias(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"what";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"what"));
}

static int
test_utils__tokcpy__unchanged(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (assert_wstring_equals(input, L"foo")
	    && assert_wstring_equals(output, L"foo"));
}

static int
test_utils__tokcpy__with_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo/bar";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (assert_wstring_equals(input, L"foo/bar")
	    && assert_wstring_equals(output, L"foo"));
}

static int
test_utils__tokcpy__empty_string(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (assert_wstring_equals(input, L"")
	    && assert_wstring_equals(output, L""));
}

static int
test_utils__tokcpy__just_a_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"/";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (assert_wstring_equals(input, L"/")
	    && assert_wstring_equals(output, L""));
}

int
main(int argc, const char *argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "");

	RUN_TEST(test_newsgroupize_null);
	RUN_TEST(test_newsgroupize_empty);
	RUN_TEST(test_newsgroupize_one);
	RUN_TEST(test_newsgroupize_slash_one);
	RUN_TEST(test_newsgroupize_root);
	RUN_TEST(test_newsgroupize_tmp);
	RUN_TEST(test_newsgroupize_home);
	RUN_TEST(test_newsgroupize_shorthome);
	RUN_TEST(test_newsgroupize_shorthome_one_level);
	RUN_TEST(test_newsgroupize_alreadyshort);
	RUN_TEST(test_newsgroupize_trailingslash);
	RUN_TEST(test_newsgroupize_alias);

	RUN_TEST(test_quickcut_null);
	RUN_TEST(test_quickcut_empty);
	RUN_TEST(test_quickcut_one_to_one);
	RUN_TEST(test_quickcut_one_to_two);
	RUN_TEST(test_quickcut_thirty_to_ten);
	RUN_TEST(test_quickcut_ten_to_thirty);
	RUN_TEST(test_quickcut_ten_to_ten);

	RUN_TEST(test_cleancut_null);
	RUN_TEST(test_cleancut_empty);
	RUN_TEST(test_cleancut_root_to_ten);
	RUN_TEST(test_cleancut_root_to_one);
	RUN_TEST(test_cleancut_tmp_to_one);
	RUN_TEST(test_cleancut_tmp_to_three);
	RUN_TEST(test_cleancut_tmp_to_four);
	RUN_TEST(test_cleancut_tmp_to_ten);
	RUN_TEST(test_cleancut_uld_to_one);
	RUN_TEST(test_cleancut_uld_to_five);
	RUN_TEST(test_cleancut_uld_to_ten);
	RUN_TEST(test_cleancut_uld_to_eleven);

	RUN_TEST(test_aliases_none);
	RUN_TEST(test_aliases_home_alone);
	RUN_TEST(test_aliases_home_and_one);
	RUN_TEST(test_aliases_home_and_tree);
	RUN_TEST(test_aliases_five_unmatching_aliases);
	RUN_TEST(test_aliases_duplicate_aliases);
	RUN_TEST(test_aliases_too_many);
	RUN_TEST(test_aliases_find_smallest);

	RUN_TEST(test_hostname_full);
	RUN_TEST(test_hostname_short);
	RUN_TEST(test_hostname_error_no_hostname);

	RUN_TEST(test_config__process_config_line__set_no_var);
	RUN_TEST(test_config__process_config_line__alias_no_name);
	RUN_TEST(test_config__process_config_line__just_spaces);
	RUN_TEST(test_config__process_config_line__comments);
	RUN_TEST(test_config__process_config_line__set_maxlength_250);
	RUN_TEST(test_config__process_config_line__set_maxlength_bad);
	RUN_TEST(test_config__process_config_line__set_maxlength_overflow);
	RUN_TEST(test_config__process_config_line__set_maxlength_quoted);

	RUN_TEST(test_config_alias_expand_prefix);
	RUN_TEST(test_config_alias_expand_prefix_with_slash);
	RUN_TEST(test_config_alias_expand_prefix_single);
	RUN_TEST(test_config_alias_expand_prefix_no_alias);

	RUN_TEST(test_utils__tokcpy__unchanged);
	RUN_TEST(test_utils__tokcpy__with_slash);
	RUN_TEST(test_utils__tokcpy__empty_string);
	RUN_TEST(test_utils__tokcpy__just_a_slash);

	printf("%d tests (%d PASS, %d FAIL)\n", tested, passed, failed);

	if (failed) {
		return (1);
	} else {
		return (0);
	}
}
