/*
 * Copyright (c) 2009-2015 Bertrand Janin <b@janin.com>
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
#include <errno.h>

#include "main.c"
#include "utils.h"
#include "strlcpy.h"
#include "wcslcpy.h"
#include "template.h"

#define RUN_TEST(f)						\
	printf("%-60s", #f);					\
	fflush(stdout);						\
	errstr = NULL;						\
	if (f()) {						\
		printf("PASS\n");				\
		passed++;					\
	} else {						\
		printf("FAIL\n");				\
		if (strlen(details)) {				\
			puts(details);				\
			details[0] = '\0';			\
		}						\
		if (errstr != NULL) {				\
			printf("%17s%ls\n", "errstr=", errstr);	\
		}						\
		failed++;					\
	};							\
	tested++;						\

#define ALIAS_ADD(a, b)						\
	alias_add(a, b, &errstr);				\
	if (errstr != NULL) {					\
		return (1);					\
	}							\


extern wchar_t cfg_filler[FILLER_LEN];
extern int alias_count;
const wchar_t *errstr;
char details[256] = "";
char test_hostname_value[MAXHOSTNAMELEN];
int tested = 0;
int passed = 0;
int failed = 0;
int test_file_exists = 1;

/* Used in the below path_wcswd() override. */
wchar_t path_wcswd_fakepwd[MAXPATHLEN] = L"/tmp";

/*
 * Override with predictable path.
 */
void
path_wcswd(wchar_t *wcswd, size_t len, const wchar_t **errstr)
{
	(void)errstr;
	wcslcpy(wcswd, path_wcswd_fakepwd, len);
}

/*
 * Override gethostname(3) to make the hostname predictable.
 */
int
lgethostname(char *name, size_t namelen)
{
	strlcpy(name, test_hostname_value, namelen);
	return (0);
}

/*
 * Override file_exists to ensure predictable returns.
 */
int
path_is_valid(char *filepath)
{
	(void)filepath;
	return (test_file_exists);
}

/*
 * Testing assertions with detailed output.
 */
static int
assert_string_equals(const char *value, const char *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (strcmp(value, expected) == 0)
		return (2);

bad:
	snprintf(details, sizeof(details),
	    "    strings do not match:\n"
	    "           value=%s\n"
	    "        expected=%s", value, expected);
	return (0);
}

static int
assert_wstring_equals(const wchar_t *value, const wchar_t *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (wcscmp(value, expected) == 0)
		return (1);

bad:
	snprintf(details, sizeof(details),
	    "    wide strings do not match:\n"
	    "           value=%ls\n"
	    "        expected=%ls", value, expected);
	return (0);
}

static int
assert_int_equals(int value, int expected)
{
	if (value == expected)
		return (1);

	snprintf(details, sizeof(details),
	    "    ints do not match:\n"
	    "           value=%d\n"
	    "        expected=%d", value, expected);

	return (0);
}

static int
assert_size_t_equals(size_t value, size_t expected)
{
	char cvalue[64], cexpected[64];

	if (value == expected)
		return (1);

	if (value == (size_t)-1) {
		strlcpy(cvalue, "(size_t)-1", sizeof(cvalue));
	} else {
		snprintf(cvalue, sizeof(cvalue), "%lu", value);
	}

	if (expected == (size_t)-1) {
		strlcpy(cexpected, "(size_t)-1", sizeof(cexpected));
	} else {
		snprintf(cexpected, sizeof(cexpected), "%lu", expected);
	}

	snprintf(details, sizeof(details),
	    "    size_t variables do not match:\n"
	    "           value=%s\n"
	    "        expected=%s", cvalue, cexpected);

	return (0);
}

static int
assert_null(const void *p)
{
	if (p == NULL)
		return (1);

	snprintf(details, sizeof(details), "    pointer is not NULL");
	return (0);
}

/*
 * newgroupize tests
 */
static int
test_path_newsgroupize__null(void)
{
	wchar_t out[64];
	path_newsgroupize(out, NULL, 64);
	return (1);
}

static int
test_path_newsgroupize__empty(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"", 64);
	return (assert_wstring_equals(out, L""));
}

static int
test_path_newsgroupize__one(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"a", 64);
	return (assert_wstring_equals(out, L"a"));
}

static int
test_path_newsgroupize__root(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/", 64);
	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_newsgroupize__slash_one(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/a", 64);
	return (assert_wstring_equals(out, L"/a"));
}

static int
test_path_newsgroupize__tmp(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo", 64);
	return (assert_wstring_equals(out, L"/foo"));
}

static int
test_path_newsgroupize__home(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo/bar", 64);
	return (assert_wstring_equals(out, L"/f/bar"));
}

static int
test_path_newsgroupize__shortpath(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"~/foo/bar", 64);
	return (assert_wstring_equals(out, L"~/f/bar"));
}

static int
test_path_newsgroupize__shortpath_one_level(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"~/foo", 64);
	return (assert_wstring_equals(out, L"~/foo"));
}

static int
test_path_newsgroupize__alreadyshort(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/a/b/c/d/e/f/g/h/i/j", 64);
	return (assert_wstring_equals(out, L"/a/b/c/d/e/f/g/h/i/j"));
}

static int
test_path_newsgroupize__trailingslash(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo/bar/", 64);
	return (assert_wstring_equals(out, L"/f/bar/"));
}

static int
test_path_newsgroupize__alias(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"$alias/foo/bar/baz", 64);
	return (assert_wstring_equals(out, L"$alias/f/b/baz"));
}


/*
 * path_quickcut()
 */
static int
test_path_quickcut__empty(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_quickcut(out, L"", 64);
	return (assert_wstring_equals(out, L""));
}

static int
test_path_quickcut__one_to_one(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	cfg_maxpwdlen = 1;
	path_quickcut(out, L"o", 64);
	return (assert_wstring_equals(out, L"o"));
}

static int
test_path_quickcut__one_to_two(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	cfg_maxpwdlen = 2;
	path_quickcut(out, L"o", 64);
	return (assert_wstring_equals(out, L"o"));
}

static int
test_path_quickcut__thirty_to_ten(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	cfg_maxpwdlen = 10;
	path_quickcut(out, L"qwertyuiopasdfghjklzxcvbnmqwer", 64);
	return (assert_wstring_equals(out, L"...bnmqwer"));
}

static int
test_path_quickcut__ten_to_thirty(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	cfg_maxpwdlen = 30;
	path_quickcut(out, L"1234567890", 64);
	return (assert_wstring_equals(out, L"1234567890"));
}

static int
test_path_quickcut__ten_to_ten(void)
{
	wchar_t out[64];
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	cfg_maxpwdlen = 10;
	path_quickcut(out, L"1234567890", 64);
	return (assert_wstring_equals(out, L"1234567890"));
}

/*
 * path_cleancut()
 */
static int
test_path_cleancut__empty(void)
{
	wchar_t out[64];
	path_cleancut(out, L"", 64);
	return (assert_wstring_equals(out, L""));
}

static int
test_path_cleancut__root_to_ten(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/", 64);
	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_cleancut__root_to_one(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/", 64);
	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_cleancut__tmp_to_one(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/tmp", 64);
	return (assert_wstring_equals(out, L"."));
}

static int
test_path_cleancut__tmp_to_three(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 3;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/tmp", 64);
	return (assert_wstring_equals(out, L"..."));
}

static int
test_path_cleancut__tmp_to_four(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 4;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/tmp", 64);
	return (assert_wstring_equals(out, L"/tmp"));
}

static int
test_path_cleancut__tmp_to_ten(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/tmp", 64);
	return (assert_wstring_equals(out, L"/tmp"));
}

static int
test_path_cleancut__uld_to_one(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/usr/local/doc", 64);
	return (assert_wstring_equals(out, L"."));
}

static int
test_path_cleancut__uld_to_five(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 5;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/usr/local/doc", 64);
	return (assert_wstring_equals(out, L"...oc"));
}

static int
test_path_cleancut__uld_to_ten(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, L"...", FILLER_LEN);
	path_cleancut(out, L"/usr/local/doc", 64);
	return (assert_wstring_equals(out, L".../doc"));
}

static int
test_path_cleancut__uld_to_eleven(void)
{
	wchar_t out[64];
	cfg_maxpwdlen = 11;
	wcslcpy(cfg_filler, L"_", FILLER_LEN);
	path_cleancut(out, L"/usr/local/doc", 64);
	return (assert_wstring_equals(out, L"_/local/doc"));
}

/*
 * aliases tests
 */
void alias_replace(wchar_t *);

static int
test_alias__replace__none(void)
{
	wchar_t pwd[] = L"/usr/local/doc";
	alias_purge_all();
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"/usr/local/doc"));
}

static int
test_alias__replace__home_alone(void)
{
	wchar_t pwd[] = L"/home/tamentis";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~"));
}

static int
test_alias__replace__home_and_one(void)
{
	wchar_t pwd[] = L"/home/tamentis/x";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~/x"));
}

static int
test_alias__replace__home_and_tree(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/projects/stuff";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/tamentis");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~/x/projects/stuff"));
}

static int
test_alias__replace__five_unmatching_aliases(void)
{
	wchar_t pwd[] = L"/home/tamentiz/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"a1", L"/the/first/path");
	ALIAS_ADD(L"b2", L"/path/second");
	ALIAS_ADD(L"c3", L"/troisieme/chemin");
	ALIAS_ADD(L"d4", L"drole/de/chemin/quatre");
	ALIAS_ADD(L"e5", L"/home/tamentïs");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"/home/tamentiz/x/projects"));
}

static int
test_alias__replace__duplicate_aliases(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"aa", L"/home/tamentis");
	ALIAS_ADD(L"aa", L"/home/tamentis");
	ALIAS_ADD(L"aa", L"/home/tamentis");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"aa/x/projects"));
}

static int
test_alias__replace__find_smallest(void)
{
	wchar_t pwd[] = L"/home/tamentis/x/y/z/projects/prwd";
	alias_count = 0;
	ALIAS_ADD(L"bad1", L"/home/tamentis");
	ALIAS_ADD(L"bad2", L"/home");
	ALIAS_ADD(L"bad3", L"/home/tamentis/x");
	ALIAS_ADD(L"good", L"/home/tamentis/x/y/z");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"good/projects/prwd"));
}

static int
test_alias__add__too_many(void)
{
	int i;
	alias_purge_all();
	for (i = 0; i < MAX_ALIASES * 2; i++) {
		ALIAS_ADD(L"aa", L"/home/tamentis");
	}

	alias_add(L"aa", L"/home/tamentis", &errstr);
	if (errstr == NULL) {
		snprintf(details, sizeof(details),
		    "alias_add should have returned an error");
		return (0);
	}

	return (assert_wstring_equals(errstr, L"too many aliases"));
}

/*
 * config file parser test
 */
// void process_config_line(wchar_t *line, int linenum, const char **);

static int
test_config__process_config_line__set_no_var(void)
{
	wchar_t line[] = L"set";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr, L"set without variable name"));
}

static int
test_config__process_config_line__alias_no_name(void)
{
	wchar_t line[] = L"alias";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr, L"alias without name"));
}

static int
test_config__process_config_line__just_spaces(void)
{
	wchar_t line[] = L"        ";
	process_config_line(line, &errstr);
	return (assert_null(errstr));
}

static int
test_config__process_config_line__comments(void)
{
	wchar_t line[] = L"# comments";
	process_config_line(line, &errstr);
	return (assert_null(errstr));
}

static int
test_config__process_config_line__set_maxlength_250(void)
{
	wchar_t line[] = L"set maxlength 250";
	process_config_line(line, &errstr);
	return (assert_null(errstr) &&
	    assert_int_equals(cfg_maxpwdlen, 250));
}

static int
test_config__process_config_line__set_maxlength_bad(void)
{
	wchar_t line[] = L"set maxlength $F@#$";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr,
	    L"invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_overflow(void)
{
	wchar_t line[] = L"set maxlength 5000";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr,
	    L"invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_quoted(void)
{
	wchar_t line[] = L"set maxlength \"50\"";
	process_config_line(line, &errstr);
	return (
	    assert_null(errstr) &&
	    assert_int_equals(cfg_maxpwdlen, 50)
	);
}

/*
 * alias_expand_prefix tests
 */
static int
test_alias__expand_prefix__normal(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what/the/fsck";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/anything/giving/the/fsck"));
}

static int
test_alias__expand_prefix__with_slash(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what/the/fsck/";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving/");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/anything/giving//the/fsck/"));
}

static int
test_alias__expand_prefix__single(void)
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
test_alias__expand_prefix__no_alias(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"what";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$what", L"/anything/giving");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"what"));
}

/* utils.c - tokcpy() tests */
static int
test_utils__tokcpy__unchanged(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"foo") &&
	    assert_wstring_equals(output, L"foo")
	);
}

static int
test_utils__tokcpy__with_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo/bar";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"foo/bar") &&
	    assert_wstring_equals(output, L"foo")
	);
}

static int
test_utils__tokcpy__empty_string(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"") &&
	    assert_wstring_equals(output, L"")
	);
}

static int
test_utils__tokcpy__just_a_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"/";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"/") &&
	    assert_wstring_equals(output, L"")
	);
}

static int
test_template_tokenize__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__one_static(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_wstring_equals(tokens[0].value, L"foo") &&
	    assert_int_equals(tokens[0].type, TOKEN_STATIC) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__one_dynamic(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"${bar}";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_wstring_equals(tokens[0].value, L"bar") &&
	    assert_int_equals(tokens[0].type, TOKEN_DYNAMIC) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__complex(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo ${bar} and fooba${r}";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 4) &&
	    assert_wstring_equals(tokens[0].value, L"foo ") &&
	    assert_wstring_equals(tokens[1].value, L"bar") &&
	    assert_wstring_equals(tokens[2].value, L" and fooba") &&
	    assert_wstring_equals(tokens[3].value, L"r") &&
	    assert_int_equals(tokens[0].type, TOKEN_STATIC) &&
	    assert_int_equals(tokens[1].type, TOKEN_DYNAMIC) &&
	    assert_int_equals(tokens[2].type, TOKEN_STATIC) &&
	    assert_int_equals(tokens[3].type, TOKEN_DYNAMIC) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__too_many_tokens(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo ${bar} and fooba${r}";
	struct token tokens[2];
	int i;

	i = template_tokenize(input, tokens, 2, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr, L"tokenize error: too many tokens")
	);
}

static int
test_template_tokenize__token_too_long(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foobarfoobarfoobarfoobarfoobarfo"\
					L"obarfoobarfoobarfoobarfoobarfoob"\
					L"arfoobarfoobarfoobarfoobarfoobar";
	struct token tokens[2];
	int i;

	i = template_tokenize(input, tokens, 2, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr,
		L"tokenize error: invalid token size")
	);
}

static int
test_template_render__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	wchar_t output[MAX_OUTPUT_LEN];
	int i;

	i = template_render(input, output, MAX_OUTPUT_LEN, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(output, L"")
	);
}

static int
test_template_render__just_static(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo bar and foobaz";
	wchar_t output[MAX_OUTPUT_LEN];
	int i;

	i = template_render(input, output, MAX_OUTPUT_LEN, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(output, L"foo bar and foobaz")
	);
}

static int
test_template_arglist__init(void)
{
	struct arglist al;

	template_arglist_init(&al);

	return (assert_int_equals(al.argc, 0));
}

static int
test_template_arglist__insert_one(void)
{
	struct arglist al;
	size_t argc;

	template_arglist_init(&al);
	argc = template_arglist_insert(&al, L"foo");

	return (
	    assert_int_equals(argc, 1) &&
	    assert_int_equals(al.argc, 1) &&
	    assert_wstring_equals(al.argv[0], L"foo")
	);
}

static int
test_template_arglist__insert_many(void)
{
	struct arglist al;
	size_t argc;

	template_arglist_init(&al);
	argc = template_arglist_insert(&al, L"foo");
	argc = template_arglist_insert(&al, L"bar");
	argc = template_arglist_insert(&al, L"baz");

	return (
	    assert_int_equals(argc, 3) &&
	    assert_int_equals(al.argc, 3) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar") &&
	    assert_wstring_equals(al.argv[2], L"baz")
	);
}

static int
test_template_arglist__insert_err_too_many_args(void)
{
	struct arglist al;
	size_t argc;
	wchar_t s[] = L"foobar0";
	int i, a;

	template_arglist_init(&al);

	/* Add just enough arguments to border the limit */
	for (i = 0; i < MAX_ARG_COUNT; i++) {
		argc = template_arglist_insert(&al, s);
		if (argc == (size_t)-1) {
			return (0);
		}
	}

	a = (
	    assert_int_equals(argc, MAX_ARG_COUNT) &&
	    assert_int_equals(al.argc, MAX_ARG_COUNT) &&
	    assert_wstring_equals(al.argv[0], s) &&
	    assert_wstring_equals(al.argv[MAX_ARG_COUNT - 1], s)
	);

	if (!a) {
		return a;
	}

	/* This final insert should cause an error. */
	argc = template_arglist_insert(&al, s);
	return (assert_size_t_equals(argc, (size_t)-1));
}

static int
test_template_arglist__insert_err_too_many_chars(void)
{
	struct arglist al;
	size_t i, argc, max;
	wchar_t s[] = L"foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" \
		      L"foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar";
	int a;

	template_arglist_init(&al);

	max = MAX_ARGLIST_SIZE / (wcslen(s) + 1);

	for (i = 0; i < max; i++) {
		argc = template_arglist_insert(&al, s);
		if (argc == (size_t)-1) {
			break;
		}
	}

	a = (
	    assert_size_t_equals(argc, max) &&
	    assert_size_t_equals(al.argc, max) &&
	    assert_wstring_equals(al.argv[0], s) &&
	    assert_wstring_equals(al.argv[max - 1], s)
	);

	if (!a) {
		return a;
	}

	/* This final insert should cause an error. */
	argc = template_arglist_insert(&al, s);
	return (assert_size_t_equals(argc, (size_t)-1));
}

static int
test_template_variable_lexer__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	int i;
	struct arglist al;

	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr)
	);
}

static int
test_template_variable_lexer__one(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foobar";
	int i;
	struct arglist al;

	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foobar")
	);
}

static int
test_template_variable_lexer__two(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo bar";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar")
	);
}

static int
test_template_variable_lexer__quoted_space(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar baz")
	);
}

static int
test_template_variable_lexer__quoted_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar\\\"baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar\"baz")
	);
}

static int
test_template_variable_lexer__quoted_double_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar\"\"baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"barbaz")
	);
}

static int
test_template_variable_lexer__unmatched_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar";
	size_t i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_size_t_equals(i, (size_t)-1) &&
	    assert_wstring_equals(errstr, L"unmatched quote")
	);
}

static int
test_template_variable_lexer__large_arg(void)
{
	wchar_t input[MAX_OUTPUT_LEN + 1];
	size_t i;
	struct arglist al;

	wmemset(input, L'?', MAX_OUTPUT_LEN + 1);
	input[MAX_OUTPUT_LEN] = L'\0';

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return assert_size_t_equals(i, 1);
}

static int
test_template_variable_lexer__err_arg_size(void)
{
	wchar_t input[MAX_ARGLIST_SIZE * 2 + 1];
	size_t i;
	struct arglist al;

	wmemset(input, L'?', MAX_ARGLIST_SIZE * 2 + 1);
	input[MAX_ARGLIST_SIZE * 2] = L'\0';

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_size_t_equals(i, (size_t)-1) &&
	    assert_wstring_equals(errstr, L"argument list too large")
	);
}

static int
test_template_variable_lexer__err_too_many(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a ";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr, L"argument list too large")
	);
}

static int
test_path_exec__path_n(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"path -n";
	wchar_t buf[MAX_OUTPUT_LEN];
	struct arglist al;

	wcslcpy(path_wcswd_fakepwd, L"/usr/local/bin", MAXPATHLEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	path_exec(al.argc, al.argv, buf, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(buf, L"/u/l/bin"));
}

static int
test_path_exec__path(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"path";
	wchar_t out[MAX_OUTPUT_LEN];
	struct arglist al;

	wcslcpy(path_wcswd_fakepwd, L"/usr/local/bin", MAXPATHLEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	path_exec(al.argc, al.argv, out, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(out, L"/usr/local/bin"));
}

static int
test_hostname_exec__short(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"hostname";
	wchar_t buf[MAX_OUTPUT_LEN];
	struct arglist al;

	strlcpy(test_hostname_value, "foobar.example.com", MAXHOSTNAMELEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	hostname_exec(al.argc, al.argv, buf, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(buf, L"foobar"));
}

static int
test_hostname_exec__long(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"hostname -l";
	wchar_t buf[MAX_OUTPUT_LEN];
	struct arglist al;

	strlcpy(test_hostname_value, "foobar.example.com", MAXHOSTNAMELEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	hostname_exec(al.argc, al.argv, buf, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(buf, L"foobar.example.com"));
}


int
main(int argc, const char *argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "");

	RUN_TEST(test_path_newsgroupize__null);
	RUN_TEST(test_path_newsgroupize__empty);
	RUN_TEST(test_path_newsgroupize__one);
	RUN_TEST(test_path_newsgroupize__slash_one);
	RUN_TEST(test_path_newsgroupize__root);
	RUN_TEST(test_path_newsgroupize__tmp);
	RUN_TEST(test_path_newsgroupize__home);
	RUN_TEST(test_path_newsgroupize__shortpath);
	RUN_TEST(test_path_newsgroupize__shortpath_one_level);
	RUN_TEST(test_path_newsgroupize__alreadyshort);
	RUN_TEST(test_path_newsgroupize__trailingslash);
	RUN_TEST(test_path_newsgroupize__alias);

	RUN_TEST(test_path_quickcut__empty);
	RUN_TEST(test_path_quickcut__one_to_one);
	RUN_TEST(test_path_quickcut__one_to_two);
	RUN_TEST(test_path_quickcut__thirty_to_ten);
	RUN_TEST(test_path_quickcut__ten_to_thirty);
	RUN_TEST(test_path_quickcut__ten_to_ten);

	RUN_TEST(test_path_cleancut__empty);
	RUN_TEST(test_path_cleancut__root_to_ten);
	RUN_TEST(test_path_cleancut__root_to_one);
	RUN_TEST(test_path_cleancut__tmp_to_one);
	RUN_TEST(test_path_cleancut__tmp_to_three);
	RUN_TEST(test_path_cleancut__tmp_to_four);
	RUN_TEST(test_path_cleancut__tmp_to_ten);
	RUN_TEST(test_path_cleancut__uld_to_one);
	RUN_TEST(test_path_cleancut__uld_to_five);
	RUN_TEST(test_path_cleancut__uld_to_ten);
	RUN_TEST(test_path_cleancut__uld_to_eleven);

	RUN_TEST(test_alias__replace__none);
	RUN_TEST(test_alias__replace__home_alone);
	RUN_TEST(test_alias__replace__home_and_one);
	RUN_TEST(test_alias__replace__home_and_tree);
	RUN_TEST(test_alias__replace__five_unmatching_aliases);
	RUN_TEST(test_alias__replace__duplicate_aliases);
	RUN_TEST(test_alias__replace__find_smallest);

	RUN_TEST(test_alias__add__too_many);

	RUN_TEST(test_alias__expand_prefix__normal);
	RUN_TEST(test_alias__expand_prefix__with_slash);
	RUN_TEST(test_alias__expand_prefix__single);
	RUN_TEST(test_alias__expand_prefix__no_alias);

	RUN_TEST(test_config__process_config_line__set_no_var);
	RUN_TEST(test_config__process_config_line__alias_no_name);
	RUN_TEST(test_config__process_config_line__just_spaces);
	RUN_TEST(test_config__process_config_line__comments);
	RUN_TEST(test_config__process_config_line__set_maxlength_250);
	RUN_TEST(test_config__process_config_line__set_maxlength_bad);
	RUN_TEST(test_config__process_config_line__set_maxlength_overflow);
	RUN_TEST(test_config__process_config_line__set_maxlength_quoted);

	RUN_TEST(test_utils__tokcpy__unchanged);
	RUN_TEST(test_utils__tokcpy__with_slash);
	RUN_TEST(test_utils__tokcpy__empty_string);
	RUN_TEST(test_utils__tokcpy__just_a_slash);

	RUN_TEST(test_template_tokenize__empty);
	RUN_TEST(test_template_tokenize__one_static);
	RUN_TEST(test_template_tokenize__one_dynamic);
	RUN_TEST(test_template_tokenize__complex);
	RUN_TEST(test_template_tokenize__too_many_tokens);
	RUN_TEST(test_template_tokenize__token_too_long);

	RUN_TEST(test_template_render__empty);
	RUN_TEST(test_template_render__just_static);

	RUN_TEST(test_template_arglist__init);
	RUN_TEST(test_template_arglist__insert_one);
	RUN_TEST(test_template_arglist__insert_many);
	RUN_TEST(test_template_arglist__insert_err_too_many_args);
	RUN_TEST(test_template_arglist__insert_err_too_many_chars);

	RUN_TEST(test_template_variable_lexer__empty);
	RUN_TEST(test_template_variable_lexer__one);
	RUN_TEST(test_template_variable_lexer__two);
	RUN_TEST(test_template_variable_lexer__quoted_space);
	RUN_TEST(test_template_variable_lexer__quoted_quote);
	RUN_TEST(test_template_variable_lexer__quoted_double_quote);
	RUN_TEST(test_template_variable_lexer__unmatched_quote);
	RUN_TEST(test_template_variable_lexer__large_arg);
	RUN_TEST(test_template_variable_lexer__err_arg_size);
	RUN_TEST(test_template_variable_lexer__err_too_many);

	RUN_TEST(test_path_exec__path);
	RUN_TEST(test_path_exec__path_n);

	RUN_TEST(test_hostname_exec__short);
	RUN_TEST(test_hostname_exec__long);

	printf("%d tests (%d PASS, %d FAIL)\n", tested, passed, failed);

	if (failed) {
		return (1);
	} else {
		return (0);
	}
}
