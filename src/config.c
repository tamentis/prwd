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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "alias.h"
#include "config.h"
#include "prwd.h"
#include "strdelim.h"
#include "utils.h"
#include "wcslcpy.h"
#include "wcstonum.h"

int 	 cfg_cleancut = 0;
size_t	 cfg_maxpwdlen = MAXPWD_LEN;
int	 cfg_mercurial = 0;
int	 cfg_git = 0;
int	 cfg_hostname = 0;
int	 cfg_uid_indicator = 0;
int	 cfg_newsgroup = 0;
wchar_t	 cfg_filler[FILLER_LEN] = FILLER_DEF;
wchar_t	 cfg_template[MAX_OUTPUT_LEN] = L"";

extern wchar_t	 home[MAXPATHLEN];

#define GET_BOOLEAN(v) (v != NULL && *v == 'o') ? 1 : 0

/*
 * Sets the value of the given variable in our global variables doing some
 * minimal type check. If any error occurs, the *errstrp pointer is set to
 * an error string, it is set to NULL otherwise.
 */
static void
set_variable(wchar_t *name, wchar_t *value, const wchar_t **errstrp)
{
	*errstrp = NULL;

	/* set template <string> */
	if (wcscmp(name, L"template") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_template = L'\0';
			return;
		}
		wcslcpy(cfg_template, value, MAX_OUTPUT_LEN);

	/* set maxlength <int> */
	} else if (wcscmp(name, L"maxlength") == 0) {
		if (value == NULL || *value == L'\0') {
			*errstrp = L"no value for set maxlength";
			return;
		}

		cfg_maxpwdlen = wcstonum(value, 1, 255, errstrp);
		if (cfg_maxpwdlen == 0) {
			*errstrp = L"invalid number for set maxlength";
			return;
		}

	/* set filler <string> */
	} else if (wcscmp(name, L"filler") == 0) {
		if (value == NULL || *value == L'\0') {
			*cfg_filler = L'\0';
			return;
		}
		wcslcpy(cfg_filler, value, FILLER_LEN);

	/* set cleancut <bool> */
	} else if (wcscmp(name, L"cleancut") == 0) {
		cfg_cleancut = GET_BOOLEAN(value);

	/* set mercurial <bool> */
	} else if (wcscmp(name, L"mercurial") == 0) {
		cfg_mercurial = GET_BOOLEAN(value);

	/* set git <bool> */
	} else if (wcscmp(name, L"git") == 0) {
		cfg_git = GET_BOOLEAN(value);

	/* set hostname <bool> */
	} else if (wcscmp(name, L"hostname") == 0) {
		cfg_hostname = GET_BOOLEAN(value);

	/* set uid_indicator <bool> */
	} else if (wcscmp(name, L"uid_indicator") == 0) {
		cfg_uid_indicator = GET_BOOLEAN(value);

	/* set newsgroup <bool> */
	} else if (wcscmp(name, L"newsgroup") == 0) {
		cfg_newsgroup = GET_BOOLEAN(value);

	/* Unknown variable */
	} else {
		*errstrp = L"unknown variable for set";
	}
}

/*
 * Parse a single line of the configuration file.  If any error occurs, the
 * errstrp pointer is set to the error message, else it is set to NULL.
 */
void
process_config_line(wchar_t *line, const wchar_t **errstrp)
{
	int len;
	wchar_t *keyword, *name, *value;

	*errstrp = NULL;

        /* Strip trailing whitespace */
	for (len = wcslen(line) - 1; len > 0; len--) {
		if (wcschr(WHITESPACE, line[len]) == NULL)
			break;
		line[len] = '\0';
	}

	/* Get the keyword. (Each line is supposed to begin with a keyword). */
	if ((keyword = strdelim(&line)) == NULL)
		return;

	/* Ignore leading whitespace. */
	if (*keyword == '\0')
		keyword = strdelim(&line);

	/* Skip blank lines and commented lines. */
	if (keyword == NULL || *keyword == '\0' ||
	    *keyword == '\n' || *keyword == '#')
		return;

	/* set varname value */
	if (wcscmp(keyword, L"set") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			*errstrp = L"set without variable name";
			return;
		}
		value = strdelim(&line);
		set_variable(name, value, errstrp);
		return;

	/* alias short long */
	} else if (wcscmp(keyword, L"alias") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			*errstrp = L"alias without name";
			return;
		}
		value = strdelim(&line);
		if (!wc_path_is_valid(value))
			return;
		alias_add(name, value, errstrp);
		if (*errstrp != NULL) {
			return;
		}

	} else {
		*errstrp = L"unknown command";
	}
}

/*
 * Open the file and feed each line one by one to process_config_line.
 */
void
read_config()
{
	FILE *fp;
	char line[128], path[MAXPATHLEN];
	const wchar_t *errstr;
	wchar_t wline[128];
	int linenum = 1;

	snprintf(path, MAXPATHLEN, "%ls/.prwdrc", home);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(wline, &errstr);
		if (errstr != NULL) {
			errx(1, "prwdrc:%d: %ls", linenum, errstr);
		}
		linenum++;
	}

	fclose(fp);
}
