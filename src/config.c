/*
 * Copyright (c) 2009-2013 Bertrand Janin <b@janin.com>
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
#include <sys/types.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <locale.h>
#include <inttypes.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "strdelim.h"
#include "utils.h"


int 	 cfg_cleancut = 0;
int	 cfg_maxpwdlen = MAXPWD_LEN;
int	 cfg_mercurial = 0;
int	 cfg_git = 0;
int	 cfg_hostname = 0;
int	 cfg_uid_indicator = 0;
int	 cfg_newsgroup = 0;
wchar_t	 cfg_filler[FILLER_LEN] = FILLER_DEF;
int	 alias_count = 0;

extern wchar_t	 home[MAXPATHLEN];


#define get_boolean(v) (v != NULL && *v == 'o') ? 1 : 0


/*
 * Add a new alias to the list.
 */
void
add_alias(wchar_t *name, wchar_t *value, int linenum)
{
	if (wcslen(value) > (MAXPATHLEN - 1)) {
		fatal("prwdrc:%d: alias path is too long (MAXPATHLEN=%d).\n",
				linenum, MAXPATHLEN);
		return;
	}

	if (wcslen(value) < wcslen(name)) {
		fatal("prwdrc:%d: alias name should not be longer than the "
				"value.\n", linenum);
		return;
	}

	if (wcschr(name, '/') != NULL) {
		fatal("prwdrc:%d: alias name should not contain any '/' "
				"(slash).\n", linenum);
		return;
	}

	if (alias_count >= MAX_ALIASES - 1) {
		fatal("prwdrc:%d: you have reached the %d aliases limit.\n",
				linenum, MAX_ALIASES);
		return;
	}

	wcslcpy(aliases[alias_count].name, name, ALIAS_NAME_LEN);
	wcslcpy(aliases[alias_count].path, value, MAXPATHLEN);
	alias_count++;
}


/*
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
void
set_variable(wchar_t *name, wchar_t *value, int linenum)
{
	/* set maxlength <int> */
	if (wcscmp(name, L"maxlength") == 0) {
		if (value == NULL || *value == '\0') {
			cfg_maxpwdlen = 0;
			return;
		}
		cfg_maxpwdlen = wcstoumax(value, NULL, 10);
		if (cfg_maxpwdlen > 255)
			fatal("prwdrc: invalid number for set maxlength.\n");

	/* set filler <string> */
	} else if (wcscmp(name, L"filler") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_filler = '\0';
			return;
		}
		wcslcpy(cfg_filler, value, FILLER_LEN);

	/* set cleancut <bool> */
	} else if (wcscmp(name, L"cleancut") == 0) {
		cfg_cleancut = get_boolean(value);

	/* set mercurial <bool> */
	} else if (wcscmp(name, L"mercurial") == 0) {
		cfg_mercurial = get_boolean(value);

	/* set git <bool> */
	} else if (wcscmp(name, L"git") == 0) {
		cfg_git = get_boolean(value);

	/* set hostname <bool> */
	} else if (wcscmp(name, L"hostname") == 0) {
		cfg_hostname = get_boolean(value);

	/* set uid_indicator <bool> */
	} else if (wcscmp(name, L"uid_indicator") == 0) {
		cfg_uid_indicator = get_boolean(value);

	/* set newsgroup <bool> */
	} else if (wcscmp(name, L"newsgroup") == 0) {
		cfg_newsgroup = get_boolean(value);

	/* ??? */
	} else {
		fatal("prwdrc: unknown variable for set on line %d.\n", linenum);
	}
}


/*
 * Parse a single line of the configuration file. Returns 0 on success or
 * anything else if an error occurred, it will be rare since most fatal errors
 * will quit the program with an error message anyways.
 */
int
process_config_line(wchar_t *line, int linenum)
{
	int len;
	wchar_t *keyword, *name, *value;

        /* Strip trailing whitespace */
	for (len = wcslen(line) - 1; len > 0; len--) {
		if (wcschr(WHITESPACE, line[len]) == NULL)
			break;
		line[len] = '\0';
	}

	/* Get the keyword. (Each line is supposed to begin with a keyword). */
	if ((keyword = strdelim(&line)) == NULL)
		return 0;

	/* Ignore leading whitespace. */
	if (*keyword == '\0')
		keyword = strdelim(&line);

	if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
		return 0;

	/* set varname value */
	if (wcscmp(keyword, L"set") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			fatal("prwdrc: set without variable name on line %d.\n", linenum);
			return -1;
		}
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* alias short long */
	} else if (wcscmp(keyword, L"alias") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			fatal("prwdrc: alias without name on line %d.\n", linenum);
			return -1;
		}
		value = strdelim(&line);
		add_alias(name, value, linenum);

	/* Unknown operation... God help us. */
	} else {
		fatal("prwdrc: unknown command on line %d.\n", linenum);
		return -1;
	}

	return 0;
}


/*
 * Open the file and feed each line one by one to process_config_line.
 */
void
read_config()
{
	FILE *fp;
	char line[128];
	wchar_t wline[128];
	int linenum = 1;
	char path[MAXPATHLEN];

	snprintf(path, MAXPATHLEN, "%ls/.prwdrc", home);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(wline, linenum++);
	}

	fclose(fp);
}

