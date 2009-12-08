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

#include <sys/param.h>
#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <locale.h>
#include <inttypes.h>

#define FILLER_LEN	16	// maximum filler length
#define FILLER_DEF	L"..."	// default filler
#define MAXPWD_LEN	24	// default maximum length
#define MAX_ALIASES	64	// maximum number of aliases
#define ALIAS_NAME_LEN	32	// size of an alias

#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""

int	 cfg_maxpwdlen = MAXPWD_LEN;
int 	 cfg_cleancut = 0;
int	 cfg_newsgroup = 0;
wchar_t	 cfg_filler[FILLER_LEN] = FILLER_DEF;
wchar_t	 home[MAXPATHLEN];
int	 alias_count = 0;

struct {
	wchar_t	name[ALIAS_NAME_LEN];
	wchar_t	path[MAXPATHLEN];
} aliases[MAX_ALIASES];


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *dst, const wchar_t *src, size_t siz);
wchar_t		*strdelim(wchar_t **s);


/**
 * Panic exit, preferably running in walls and waving your arms over your head.
 */
#ifndef TESTING
void
fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(-1);
}
#else
void fatal(const char *fmt, ...);
#endif


/**
 * Add a new alias to the list.
 */
void
add_alias(wchar_t *name, wchar_t *value, int linenum)
{
	if (wcslen(value) < wcslen(name)) {
		fatal("prwdrc: alias name should not be longer than the value.\n");
		return;
	}
	if (wcschr(name, '/') != NULL) {
		fatal("prwdrc: alias name should not contain any '/' (slash).\n");
		return;
	}
	if (alias_count >= MAX_ALIASES - 1) {
		fatal("prwdrc: you cannot have more than %d aliases.\n", MAX_ALIASES);
		return;
	}
	wcslcpy(aliases[alias_count].name, name, ALIAS_NAME_LEN);
	wcslcpy(aliases[alias_count].path, value, MAXPATHLEN);
	alias_count++;
}


/**
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
void
set_variable(wchar_t *name, wchar_t *value, int linenum)
{
	/* set maxlength <int> */
	if (wcscasecmp(name, L"maxlength") == 0) {
		if (value == NULL || *value == '\0') {
			cfg_maxpwdlen = 0;
			return;
		}
		cfg_maxpwdlen = wcstoumax(value, NULL, 10);
		if (cfg_maxpwdlen > 255)
			fatal("prwdrc: invalid number for set maxlength.\n");

	/* set filler <string> */
	} else if (wcscasecmp(name, L"filler") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_filler = '\0';
			return;
		}
		wcslcpy(cfg_filler, value, FILLER_LEN);

	/* set cleancut <bool> */
	} else if (wcscasecmp(name, L"cleancut") == 0) {
		cfg_cleancut = (value != NULL && *value == 'o') ? 1 : 0;

	/* set newsgroup <bool> */
	} else if (wcscasecmp(name, L"newsgroup") == 0) {
		cfg_newsgroup = (value != NULL && *value == 'o') ? 1 : 0;

	/* ??? */
	} else {
		fatal("prwdrc: unknown variable for set on line %d.\n", linenum);
	}
}


/**
 * Parse a single line of the config file. Returns 0 on success or
 * anything else if an error occured, it will be rare since most
 * fatal errors will quit the program with an error message
 * anyways.
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
	if (wcscasecmp(keyword, L"set") == 0) {
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


/**
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


/**
 * Take a string and replace all the full words by their first
 * letters, except the last one.
 */
void
newsgroupize(wchar_t *s)
{
	wchar_t t[MAXPATHLEN];
	wchar_t *last = NULL, *org = s;
	int idx = 0;

	if (s == NULL || *s == '\0')
		return;

	/* Unless we are starting from a / (slash), we can use the first one */
	if (*s != L'/') {
		t[idx++] = *s;
	}
	t[idx++] = '/';
	t[idx] = '\0';

	/* For every component, add the first letter and a slash */
	while ((s = wcschr(s, L'/')) != NULL) {
		/* Catter for trailing slashes */
		if (s[1] == '\0')
			break;
		last = ++s;
		t[idx++] = (wchar_t)*s;
		t[idx++] = L'/';
	}
	
	/* Copy the letters+slash and make sure the last part is left untouched. */
	wcslcpy(org, t, idx);
	if (last != NULL)
		wcslcpy(org + idx - 2, last, wcslen(last) + 1);
}

/**
 * Reduce the given string with the global max length and filler.
 */
void
quickcut(wchar_t *s, size_t len)
{
	wchar_t t[MAXPATHLEN];
	size_t	filler_len = wcslen(cfg_filler), cl = sizeof(wchar_t);

	if (s == NULL || len == 0 || *s == '\0' || len <= cfg_maxpwdlen)
		return;

	wcslcpy(t, cfg_filler, filler_len + cl);
	wcslcpy(t + filler_len, s + len - cfg_maxpwdlen + filler_len, cfg_maxpwdlen - filler_len + cl);
	wcslcpy(s, t, cfg_maxpwdlen + cl);
}


/**
 * Reduce the given string to the smallest it could get to fit within
 * the global max length and without cutting any word.
 */
void
cleancut(wchar_t *s)
{
	size_t flen;
	wchar_t *last = NULL, t[MAXPATHLEN], *org = s;

	/* NULL or empty input, nothing to touch */
	if (s == NULL || *s == '\0')
		return;

	/* Nothing needs to be cropped */
	if (wcslen(s) <= cfg_maxpwdlen)
		return;

	/* As long as we can't fit 's' within the maxpwdlen, keep trimming */
	flen = wcslen(cfg_filler);
	while (wcslen(s) > (cfg_maxpwdlen - flen)) {
		s++;
		s = wcschr(s, '/');
		if (s == NULL)
			break;
		last = s;
	}

	/* The last element was too long, keep it */
	if (s == NULL) {
		/* 
		 * last has never been touched, this means we only have
		 * one slash, revert s to its original value, there is
		 * nothing we can crop.
		 */
		if (last == NULL) {
			s = org;
			goto cleancut_final;
		} else {
			s = last;
		}
	}

	s -= flen;
	wcsncpy(s, cfg_filler, flen);
	
cleancut_final:
	wcslcpy(t, s, MAXPATHLEN);
	wcslcpy(org, t, MAXPATHLEN);
}

/**
 * Loop through the user-defined aliases and find the best match to
 * get the shortest path as possible.
 */
void
replace_aliases(wchar_t *s)
{
	int i, chosen = -1;
	size_t len, nlen, max = 0;
	wchar_t t[MAXPATHLEN], *org = s;

	for (i = 0; i < alias_count; i++) {
		len = wcslen(aliases[i].path);
		if (wcsncmp(aliases[i].path, s, len) == 0) {
			if (len > max) {
				chosen = i;
				max = len;
			}
		}
	}

	/* No alias found, you can leave now */
	if (i == -1)
		return;
	
	nlen = wcslen(aliases[chosen].name);
	s += (max - nlen);
	wcsncpy(s, aliases[chosen].name, nlen);

	wcslcpy(t, s, MAXPATHLEN);
	wcslcpy(org, t, MAXPATHLEN);
}


#ifndef TESTING
int
main(int ac, const char **av)
{
	wchar_t pwd[MAXPATHLEN], *s;
	size_t len, nlen;
	char *t;

	if (ac != 1) {
		printf("usage: prwd\n");
		exit(-1);
	}

	setlocale(LC_ALL, "");

	/* Get $HOME */
	t = getenv("HOME");
	mbstowcs(home, t, MAXPATHLEN);
	if (home == NULL || *home == '\0')
		errx(0, "Unknown variable '$HOME'.");

	/* Get the working directory */
	t = getcwd(NULL, MAXPATHLEN);
	mbstowcs(pwd, t, MAXPATHLEN);
	if (pwd == NULL)
		errx(0, "Unable to get current working directory.");
	free(t);

	read_config();

	/* Alias handling */
	replace_aliases(pwd);

	/* Replace the beginning with ~ for directories within $HOME. */
	add_alias("~", home)
	/*
	len = wcslen(home);
	if (wcsncmp(home, s, len) == 0) {
		s += (len - 1);
		*s = '~';
	}
	*/

	/* Newsgroup mode, keep only the first letters. */
	if (cfg_newsgroup == 1)
		newsgroupize(pwd);

	/* If the path is still too long, crop it. */
	len = wcslen(s);

	if (cfg_maxpwdlen > 0 && len > cfg_maxpwdlen) {
		if (cfg_cleancut == 1 && cfg_newsgroup != 1) {
			cleancut(pwd);
		} else {
			quickcut(pwd, len);
		}
	}

	printf("%ls\n", pwd);

	return 0;
}
#endif
