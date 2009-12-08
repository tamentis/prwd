/* $Id$ */
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


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <stdarg.h>

extern int cfg_maxpwdlen;
extern wchar_t cfg_filler[];
extern int alias_count;
char errbuffer[255] = "";

void fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vsprintf(errbuffer, fmt, args);
	va_end(args);
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

void
test_newsgroupize_utf(void)
{
	wchar_t s[] = L"…/région/Franche-Comté";
	newsgroupize(s);
	assert(wcscmp(s, L"…/r/Franche-Comté") == 0);
}

void
test_newsgroupize()
{
	printf("newsgroupize\n");
	test_newsgroupize_null();
	test_newsgroupize_empty();
	test_newsgroupize_one();
	test_newsgroupize_home();
	test_newsgroupize_shorthome();
	test_newsgroupize_alreadyshort();
	test_newsgroupize_trailingslash();
	test_newsgroupize_utf();
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
test_quickcut_thirty_to_ten_utf(void)
{
	wchar_t s[] = L"ábcdéèfghìïjklmnóöpqrstuvwxyzÜ";
	wcscpy(cfg_filler, L"…");
	cfg_maxpwdlen = 10;
	quickcut(s, 30);
	assert(wcscmp(s, L"…stuvwxyzÜ") == 0);
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

void
test_quickcut()
{
	printf("quickcut\n");
	test_quickcut_null();
	test_quickcut_empty();
	test_quickcut_one_to_one();
	test_quickcut_one_to_two();
	test_quickcut_thirty_to_ten();
	test_quickcut_thirty_to_ten_utf();
	test_quickcut_ten_to_thirty();
	test_quickcut_ten_to_ten();
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
	cfg_maxpwdlen = 1;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"/") == 0);
}

void
test_cleancut_tmp_to_one()
{
	wchar_t s[] = L"/tmp";
	cfg_maxpwdlen = 1;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_three()
{
	wchar_t s[] = L"/tmp";
	cfg_maxpwdlen = 3;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_four()
{
	wchar_t s[] = L"/tmp";
	cfg_maxpwdlen = 4;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_tmp_to_ten()
{
	wchar_t s[] = L"/tmp";
	cfg_maxpwdlen = 10;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"/tmp") == 0);
}

void
test_cleancut_uld_to_one()
{
	wchar_t s[] = L"/usr/local/doc";
	cfg_maxpwdlen = 1;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"…/doc") == 0);
}

void
test_cleancut_uld_to_five()
{
	wchar_t s[] = L"/usr/local/doc";
	cfg_maxpwdlen = 5;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"…/doc") == 0);
}

void
test_cleancut_uld_to_ten()
{
	wchar_t s[] = L"/usr/local/doc";
	cfg_maxpwdlen = 10;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"…/doc") == 0);
}

void
test_cleancut_uld_to_eleven()
{
	wchar_t s[] = L"/usr/local/doc";
	cfg_maxpwdlen = 11;
	wcscpy(cfg_filler, L"…");
	cleancut(s);
	assert(wcscmp(s, L"…/local/doc") == 0);
}

void
test_cleancut()
{
	printf("cleancut\n");
	test_cleancut_null();
	test_cleancut_empty();
	test_cleancut_root_to_ten();
	test_cleancut_root_to_one();
	test_cleancut_tmp_to_one();
	test_cleancut_tmp_to_three();
	test_cleancut_tmp_to_four();
	test_cleancut_tmp_to_ten();
	test_cleancut_uld_to_one();
	test_cleancut_uld_to_five();
	test_cleancut_uld_to_ten();
	test_cleancut_uld_to_eleven();
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
	int max_aliases = 64; // until the #define is accessible
	alias_count = 0;
	for (i = 0; i < max_aliases + 1; i++) {
		add_alias(L"aa", L"/home/tamentis", 1);
	}
	assert(alias_count == max_aliases - 1);
	assert(strstr(errbuffer, "you cannot have more") != NULL);
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

void
test_aliases()
{
	printf("aliases\n");
	test_aliases_none();
	test_aliases_home_alone();
	test_aliases_home_and_one();
	test_aliases_home_and_tree();
	test_aliases_five_unmatching_aliases();
	test_aliases_duplicate_aliases();
	test_aliases_too_many();
	test_aliases_find_smallest();
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

void
test_config()
{
	printf("test_config\n");
	test_config_set_empty();
	test_config_alias_empty();
	test_config_many_spaces();
	test_config_comments();
	test_config_set_maxlength_250();
	test_config_set_maxlength_crap();
	test_config_set_maxlength_overflow();
	test_config_set_maxlength_quoted();
}


int
main(int argc, const char *argv[])
{
	setlocale(LC_ALL, "");

	test_newsgroupize();
	test_quickcut();
	test_cleancut();
	test_aliases();
	test_config();

	printf("all done.\n");

	return 0;
}
