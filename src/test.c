/*
 * Copyright (c) 2009 Bertrand Janin <tamentis@neopulsar.org>
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

#include "prwd.h"
#include "config.h"
#include "utils.h"


#define ERROR_BUFFER_LEN 255
#define RUN_TEST(f)		\
	printf(#f "... ");	\
	fflush(stdout);		\
	f();			\
	tested++;		\
	printf("PASS\n");

extern int cfg_maxpwdlen;
extern wchar_t cfg_filler[];
extern int alias_count;
char errbuffer[ERROR_BUFFER_LEN] = "";
char test_hostname_value[MAXHOSTNAMELEN];
int test_hostname_return = 0;
int tested = 0;


void fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vsnprintf(errbuffer, ERROR_BUFFER_LEN, fmt, args);
	va_end(args);
}


/*
 * Override insuring the hostname is predictable for the purpose of testing.
 */
int
get_full_hostname(char *buf, size_t size)
{
	strlcpy(buf, test_hostname_value, size);
	return test_hostname_return;
}


/*
 * newgroupize tests
 */
void newsgroupize(wchar_t *);

void
test_newsgroupize_null(void)
{
	newsgroupize(NULL);
}

void
test_newsgroupize_empty(void)
{
	wchar_t s[] = L"";
	newsgroupize(s);
	assert(*s == '\0');
}

void
test_newsgroupize_one(void)
{
	wchar_t s[] = L"a";
	newsgroupize(s);
	assert(wcscmp(s, L"a") == 0);
}

void
test_newsgroupize_root(void)
{
	wchar_t s[] = L"/";
	newsgroupize(s);
	assert(wcscmp(s, L"/") == 0);
}

void
test_newsgroupize_slash_one(void)
{
	wchar_t s[] = L"/a";
	newsgroupize(s);
	assert(wcscmp(s, L"/a") == 0);
}

void
test_newsgroupize_tmp(void)
{
	wchar_t s[] = L"/tmp";
	newsgroupize(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_newsgroupize_home(void)
{
	wchar_t s[] = L"/home/tamentis";
	newsgroupize(s);
	assert(wcscmp(s, L"/h/tamentis") == 0);
}

void
test_newsgroupize_shorthome(void)
{
	wchar_t s[] = L"~/projects/prwd";
	newsgroupize(s);
	assert(wcscmp(s, L"~/p/prwd") == 0);
}

void
test_newsgroupize_alreadyshort(void)
{
	wchar_t s[] = L"/a/b/c/d/e/f/g/h/i/j";
	newsgroupize(s);
	assert(wcscmp(s, L"/a/b/c/d/e/f/g/h/i/j") == 0);
}

void
test_newsgroupize_trailingslash(void)
{
	wchar_t s[] = L"/usr/local/";
	newsgroupize(s);
	assert(wcscmp(s, L"/u/local/") == 0);
}


/*
 * Quick Cut tests
 */
void	quickcut(wchar_t *, size_t);

void
test_quickcut_null(void)
{
	quickcut(NULL, 0);
}

void
test_quickcut_empty(void)
{
	wchar_t s[] = L"";
	quickcut(s, 0);
	assert(wcscmp(s, L"") == 0);
}

void
test_quickcut_one_to_one(void)
{
	wchar_t s[] = L"o";
	cfg_maxpwdlen = 1;
	quickcut(s, 1);
	assert(wcscmp(s, L"o") == 0);
}

void
test_quickcut_one_to_two(void)
{
	wchar_t s[] = L"o";
	cfg_maxpwdlen = 2;
	quickcut(s, 1);
	assert(wcscmp(s, L"o") == 0);
}

void
test_quickcut_thirty_to_ten(void)
{
	wchar_t s[] = L"qwertyuiopasdfghjklzxcvbnmqwer";
	cfg_maxpwdlen = 10;
	quickcut(s, 30);
	assert(wcscmp(s, L"...bnmqwer") == 0);
}

void
test_quickcut_ten_to_thirty(void)
{
	wchar_t s[] = L"1234567890";
	cfg_maxpwdlen = 30;
	quickcut(s, 10);
	assert(wcscmp(s, L"1234567890") == 0);
}

void
test_quickcut_ten_to_ten(void)
{
	wchar_t s[] = L"1234567890";
	cfg_maxpwdlen = 10;
	quickcut(s, 10);
	assert(wcscmp(s, L"1234567890") == 0);
}


/*
 * Clean cut tests
 */
void cleancut(wchar_t *s);

void
test_cleancut_null()
{
	cleancut(NULL);
}

void
test_cleancut_empty()
{
	wchar_t s[] = L"";
	cleancut(s);
	assert(s[0] == L'\0');
}

void
test_cleancut_root_to_ten()
{
	wchar_t s[] = L"/";
	cfg_maxpwdlen = 10;
	cleancut(s);
	assert(wcscmp(s, L"/") == 0);
}

void
test_cleancut_root_to_one()
{
	wchar_t s[] = L"/";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"/") == 0);
}

void
test_cleancut_tmp_to_one()
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_three()
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 3;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_four()
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 4;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_ten()
{
	wchar_t s[] = L"/tmp";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_uld_to_one()
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 1;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L".../doc") == 0);
}

void
test_cleancut_uld_to_five()
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 5;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L".../doc") == 0);
}

void
test_cleancut_uld_to_ten()
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"...";
	cfg_maxpwdlen = 10;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L".../doc") == 0);
}

void
test_cleancut_uld_to_eleven()
{
	wchar_t s[] = L"/usr/local/doc";
	wchar_t f[] = L"_";
	cfg_maxpwdlen = 11;
	wcslcpy(cfg_filler, f, sizeof(f));
	cleancut(s);
	assert(wcscmp(s, L"_/local/doc") == 0);
}


/*
 * aliases tests
 */
void add_alias(wchar_t *, wchar_t *, int);
void replace_aliases(wchar_t *);

void
test_aliases_none()
{
	wchar_t pwd[] = L"/usr/local/doc";
	alias_count = 0;
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"/usr/local/doc") == 0);
}

void
test_aliases_home_alone()
{
	wchar_t pwd[] = L"/home/tamentis";
	alias_count = 0;
	add_alias(L"~", L"/home/tamentis", 1);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"~") == 0);
}

void
test_aliases_home_and_one()
{
	wchar_t pwd[] = L"/home/tamentis/x";
	alias_count = 0;
	add_alias(L"~", L"/home/tamentis", 1);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"~/x") == 0);
}

void
test_aliases_home_and_tree()
{
	wchar_t pwd[] = L"/home/tamentis/x/projects/stuff";
	alias_count = 0;
	add_alias(L"~", L"/home/tamentis", 1);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"~/x/projects/stuff") == 0);
}

void
test_aliases_five_unmatching_aliases()
{
	wchar_t pwd[] = L"/home/tamentiz/x/projects";
	alias_count = 0;
	add_alias(L"a1", L"/the/first/path", 1);
	add_alias(L"b2", L"/path/second", 1);
	add_alias(L"c3", L"/troisieme/chemin", 1);
	add_alias(L"d4", L"drole/de/chemin/quatre", 1);
	add_alias(L"e5", L"/home/tamentïs", 1);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"/home/tamentiz/x/projects") == 0);
}

void
test_aliases_duplicate_aliases()
{
	wchar_t pwd[] = L"/home/tamentis/x/projects";
	alias_count = 0;
	add_alias(L"aa", L"/home/tamentis", 1);
	add_alias(L"aa", L"/home/tamentis", 2);
	add_alias(L"aa", L"/home/tamentis", 3);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"aa/x/projects") == 0);
}

void
test_aliases_too_many()
{
	int i;
	int max_aliases = MAX_ALIASES;
	alias_count = 0;
	for (i = 0; i < max_aliases + 1; i++) {
		add_alias(L"aa", L"/home/tamentis", 1);
	}
	assert(alias_count == max_aliases - 1);
	assert(strstr(errbuffer, "you have reached") != NULL);
}

void
test_aliases_find_smallest()
{
	wchar_t pwd[] = L"/home/tamentis/x/y/z/projects/prwd";
	alias_count = 0;
	add_alias(L"bad1", L"/home/tamentis", 2);
	add_alias(L"bad2", L"/home", 2);
	add_alias(L"bad3", L"/home/tamentis/x", 3);
	add_alias(L"good", L"/home/tamentis/x/y/z", 3);
	replace_aliases(pwd);
	assert(wcscmp(pwd, L"good/projects/prwd") == 0);
}


/*
 * config file parser test
 */
int process_config_line(wchar_t *line, int linenum);

void
test_config_set_empty()
{
	wchar_t line[] = L"set";
	*errbuffer = '\0';
	process_config_line(line, 0);
	assert(strstr(errbuffer, "without variable name") != NULL);
}

void
test_config_alias_empty()
{
	wchar_t line[] = L"alias";
	*errbuffer = '\0';
	process_config_line(line, 0);
	assert(strstr(errbuffer, "alias without name") != NULL);
}

void
test_config_many_spaces()
{
	wchar_t line[] = L"        ";
	*errbuffer = '\0';
	process_config_line(line, 1);
}

void
test_config_comments()
{
	wchar_t line[] = L"# comments";
	*errbuffer = '\0';
	process_config_line(line, 1);
}

void
test_config_set_maxlength_250()
{
	wchar_t line[] = L"set maxlength 250";
	*errbuffer = '\0';
	process_config_line(line, 1);
	assert(cfg_maxpwdlen == 250);
}

void
test_config_set_maxlength_crap()
{
	wchar_t line[] = L"set maxlength $F@#$";
	*errbuffer = '\0';
	process_config_line(line, 1);
	assert(cfg_maxpwdlen == 0);
}

void
test_config_set_maxlength_overflow()
{
	wchar_t line[] = L"set maxlength 5000";
	*errbuffer = '\0';
	process_config_line(line, 1);
	assert(strstr(errbuffer, "invalid number") != NULL);
}

void
test_config_set_maxlength_quoted()
{
	wchar_t line[] = L"set maxlength \"50\"";
	*errbuffer = '\0';
	process_config_line(line, 1);
	assert(cfg_maxpwdlen == 50);
}


/*
 * test the hostname feature
 */
void add_hostname(wchar_t *);

void
test_hostname_full()
{
	wchar_t s[256] = L"anything";
	char h[] = "odin.tamentis.com";

	test_hostname_return = 0;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	assert(wcscmp(s, L"odin:anything") == 0);
}

void
test_hostname_short()
{
	wchar_t s[256] = L"anything";
	char h[] = "odin";

	test_hostname_return = 0;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	assert(wcscmp(s, L"odin:anything") == 0);
}

void
test_hostname_error_no_hostname()
{
	wchar_t s[256] = L"anything";
	char h[] = "odin";

	test_hostname_return = -1;
	strlcpy(test_hostname_value, h, sizeof(h));

	add_hostname(s);
	assert(strstr(errbuffer, "gethostname() failed") != NULL);
}

/*
 * test the expand alias tools
 */
void expand_aliases(wchar_t *, int);

void
test_config_expand_aliases()
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$what/the/fsck";
	char h[] = "odin.tamentis.com";

	add_alias(L"$what", L"/anything/giving", 1);
	expand_aliases(s, MAX_OUTPUT_LEN);
	assert(wcscmp(s, L"/anything/giving/the/fsck") == 0);
}


int
main(int argc, const char *argv[])
{
	setlocale(LC_ALL, "");

	RUN_TEST(test_newsgroupize_null);
	RUN_TEST(test_newsgroupize_empty);
	RUN_TEST(test_newsgroupize_one);
	RUN_TEST(test_newsgroupize_slash_one);
	RUN_TEST(test_newsgroupize_root);
	RUN_TEST(test_newsgroupize_tmp);
	RUN_TEST(test_newsgroupize_home);
	RUN_TEST(test_newsgroupize_shorthome);
	RUN_TEST(test_newsgroupize_alreadyshort);
	RUN_TEST(test_newsgroupize_trailingslash);

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

	RUN_TEST(test_config_set_empty);
	RUN_TEST(test_config_alias_empty);
	RUN_TEST(test_config_many_spaces);
	RUN_TEST(test_config_comments);
	RUN_TEST(test_config_set_maxlength_250);
	RUN_TEST(test_config_set_maxlength_crap);
	RUN_TEST(test_config_set_maxlength_overflow);
	RUN_TEST(test_config_set_maxlength_quoted);

	RUN_TEST(test_config_expand_aliases);

	printf("%d tests\n", tested);

	return 0;
}
