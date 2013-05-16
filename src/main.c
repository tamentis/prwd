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

#define FILLER_LEN	16	// maximum filler length
#define FILLER_DEF	L"..."	// default filler
#define MAXPWD_LEN	24	// default maximum length
#define MAX_ALIASES	64	// maximum number of aliases
#define ALIAS_NAME_LEN	32	// size of an alias
#define MAX_BRANCH_LEN	32	// max size of a branch name

#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""

int 	 cfg_cleancut = 0;
int	 cfg_maxpwdlen = MAXPWD_LEN;
int	 cfg_mercurial = 0;
int	 cfg_git = 0;
int	 cfg_newsgroup = 0;
wchar_t	 cfg_filler[FILLER_LEN] = FILLER_DEF;
wchar_t	 home[MAXPATHLEN];
int	 alias_count = 0;

struct {
	wchar_t	name[ALIAS_NAME_LEN];
	wchar_t	path[MAXPATHLEN];
} aliases[MAX_ALIASES];


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *, const wchar_t *, size_t);
wchar_t		*strdelim(wchar_t **);
size_t		 strlcpy(char *, const char *, size_t);


/* Types of source control mechanisms */
enum version_control_system {
	VCS_NONE,
	VCS_MERCURIAL,
	VCS_GIT
};


/**
 * Panic exit, preferably screaming, running into walls with your arms in the
 * air.
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
		cfg_cleancut = (value != NULL && *value == 'o') ? 1 : 0;

	/* set mercurial <bool> */
	} else if (wcscmp(name, L"mercurial") == 0) {
		cfg_mercurial = (value != NULL && *value == 'o') ? 1 : 0;

	/* set git <bool> */
	} else if (wcscmp(name, L"git") == 0) {
		cfg_git = (value != NULL && *value == 'o') ? 1 : 0;

	/* set newsgroup <bool> */
	} else if (wcscmp(name, L"newsgroup") == 0) {
		cfg_newsgroup = (value != NULL && *value == 'o') ? 1 : 0;

	/* ??? */
	} else {
		fatal("prwdrc: unknown variable for set on line %d.\n", linenum);
	}
}


/**
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

	/* Root (/) can escape right now, it's as short as it can get */
	if (*s == L'/' && s[1] == '\0')
		return;

	/* Unless we are starting from a / (slash), we can use the first one */
	if (*s != L'/') {
		t[idx++] = *s;
	}

	/* Keep the first part if it's an alias (start by *) */
	if (*s == L'*') {
		wchar_t *sl = wcschr(s, L'/');

		if (sl) {
			wcslcpy(t, s, sl - s + 1);
			idx += sl - s - 1;
			s = sl;
			/* If we have no other /, keep the alias AND the last
			 * part, so we return without doing anything */
			sl = wcschr(s + 1, L'/');
			if (sl == NULL) {
				return;
			}
		}
	}
	t[idx++] = '/';
	t[idx] = '\0';

	/* For every component, add the first letter and a slash */
	while ((s = wcschr(s, L'/')) != NULL) {
		/* Cater for trailing slashes */
		if (s[1] == '\0')
			break;
		last = ++s;
		t[idx++] = (wchar_t)*s;
		t[idx++] = L'/';
	}

	/* idx is less than 4, we only have one slash, just keep org as is */
	if (idx < 4)
		return;

	/* Copy letters+slash making sure the last part is left untouched. */
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
	wcslcpy(t + filler_len, s + len - cfg_maxpwdlen + filler_len,
			cfg_maxpwdlen - filler_len + cl);
	wcslcpy(s, t, cfg_maxpwdlen + cl);
}

/**
 * Recurse up from $PWD to find a .hg/ directory with a valid branch file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 */
size_t
get_mercurial_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c;
	char pwd[MAXPATHLEN];
	char candidate[MAXPATHLEN];
	char buf[MAX_BRANCH_LEN];
	size_t branch_size;
	struct stat bufstat;
	int found_repo = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAXPATHLEN), MAXPATHLEN);

	do {
		snprintf(candidate, MAXPATHLEN, "%s/.hg/branch", pwd);

		found_repo = stat(candidate, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found_repo != 0 && candidate[1] != '\0');

	if (found_repo == -1)
		return 0;

	fp = fopen(candidate, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		return mbstowcs(dst, buf, MAX_BRANCH_LEN);
	}

	if (fread(buf, 1, size, fp) == 0)
		fatal("prwd: failed to read the .hg/branch file.\n");
	fclose(fp);

	/* remove the trailing new line if any */
	if ((c = strchr(buf, '\n')) != NULL)
		*c = '\0';

	branch_size = mbstowcs(dst, buf, MAX_BRANCH_LEN);

	return branch_size;
}

/**
 * Recurse up from $PWD to find a .git/ directory with a valid HEAD file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 */
size_t
get_git_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c;
	char pwd[MAXPATHLEN];
	char candidate[MAXPATHLEN];
	char buf[MAX_BRANCH_LEN];
	size_t s;
	struct stat bufstat;
	int found_repo = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAXPATHLEN), MAXPATHLEN);

	do {
		snprintf(candidate, MAXPATHLEN, "%s/.git/HEAD", pwd);

		found_repo = stat(candidate, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found_repo != 0 && candidate[1] != '\0');

	if (found_repo == -1)
		return 0;

	fp = fopen(candidate, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		return mbstowcs(dst, buf, MAX_BRANCH_LEN);
	}

	s = fread(buf, 1, size, fp);
	fclose(fp);

	buf[MAX_BRANCH_LEN - 1] = '\0';

	/* This is a branch head, just print the branch. */
	if (strncmp(buf, "ref: refs/heads/", 16) == 0) {
		char *nl = strchr(buf, '\n');
		if (nl)
			*(nl) = '\0';
		c = buf + 16;
		return mbstowcs(dst, c, MAX_BRANCH_LEN);
	}

	/* Show all other kinds of ref as-is (does it even exist?) */
	if (strncmp(buf, "ref:", 4) == 0) {
		char *nl = strchr(buf, '\n');
		if (nl) 
			*(nl) = '\0';
		c = buf + 5;
		return mbstowcs(dst, c, MAX_BRANCH_LEN);
	}

	/* That's probably just a changeset, just show the first 6 chars */
	if (s > 6) {
		strlcpy(buf + 6, "...", 4);
		return mbstowcs(dst, buf, MAX_BRANCH_LEN);
	}

	/* We shouldn't get there, but we mind as well no crash. */
	strlcpy(buf, "???", 4);
	return mbstowcs(dst, buf, MAX_BRANCH_LEN);
}

/**
 * Add the mercurial branch at the beginning of the path. If a branch was
 * found, 1 is returned else 0.
 */
int
add_branch(wchar_t *s, enum version_control_system vcs)
{
	wchar_t org[MAXPATHLEN];
	wchar_t branch[MAX_BRANCH_LEN];
	size_t len;

	wcslcpy(org, s, MAXPATHLEN);

	switch (vcs) {
		case VCS_MERCURIAL:
			if (get_mercurial_branch(branch, MAX_BRANCH_LEN) == 0)
				return 0;
			break;
		case VCS_GIT:
			if (get_git_branch(branch, MAX_BRANCH_LEN) == 0)
				return 0;
			break;
		default:
			return 0;
	}

	len = wcslcpy(s, branch, MAX_BRANCH_LEN);

	s += len;
	*(s++) = ':';
	wcslcpy(s, org, MAXPATHLEN - len - 1);

	return 1;
}


/**
 * Reduce the given string to the smallest it could get to fit within
 * the global max length and without cutting any word.
 */
void
cleancut(wchar_t *s)
{
	int flen;
	wchar_t *last = NULL, t[MAXPATHLEN], *org = s;

	/* NULL or empty input, nothing to touch */
	if (s == NULL || *s == '\0')
		return;

	/* Nothing needs to be cropped */
	if (wcslen(s) <= cfg_maxpwdlen)
		return;

	/* As long as we can't fit 's' within the maxpwdlen, keep trimming */
	flen = wcslen(cfg_filler);
	while ((int)wcslen(s) > (cfg_maxpwdlen - flen)) {
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
	if (chosen == -1)
		return;

	nlen = wcslen(aliases[chosen].name);
	s += (max - nlen);
	wcsncpy(s, aliases[chosen].name, nlen);

	wcslcpy(t, s, MAXPATHLEN);
	wcslcpy(org, t, MAXPATHLEN);
}


void
show_version(void)
{
	puts("prwd-" VERSION);
}


/*
 * Dump all the aliases starting with $ as shell variable. This output is meant
 * to be used with eval in your profile file.
 *
 * This thing could be doing some shell escaping on the path, but I'd be
 * surprised anyone using this software would have such monstrosities on their
 * computers.
 */
void
dump_alias_vars(void)
{
	int i;

	for (i = 0; i < alias_count; i++) {
		if (aliases[i].name[0] == '$') {
			wprintf(L"export %ls=\"%ls\"\n", aliases[i].name + 1,
					aliases[i].path);
		}
	}
}


/*
 * Main prwd functionality, prints a reduced working directory.
 */
void
prwd(void)
{
	size_t len;
	int found_repo = 0;
	char *t;
	char mbpwd[MAXPATHLEN];
	wchar_t pwd[MAXPATHLEN];

	/* Get the working directory */
	t = getcwd(NULL, MAXPATHLEN);
	if (t == NULL)
		errx(0, "Unable to get current working directory.");
	mbstowcs(pwd, t, MAXPATHLEN);
	free(t);

	/* Replace the beginning with ~ for directories within $HOME. */
	add_alias(L"~", home, 0);

	/* Alias handling */
	replace_aliases(pwd);

	/* Newsgroup mode, keep only the first letters. */
	if (cfg_newsgroup == 1)
		newsgroupize(pwd);

	/* If the path is still too long, crop it. */
	len = wcslen(pwd);

	if (cfg_maxpwdlen > 0 && len > cfg_maxpwdlen) {
		if (cfg_cleancut == 1 && cfg_newsgroup != 1) {
			cleancut(pwd);
		} else {
			quickcut(pwd, len);
		}
	}

	/* If mercurial is enabled, show the branch */
	if (cfg_mercurial == 1) {
		found_repo = add_branch(pwd, VCS_MERCURIAL);
	}

	if (found_repo == 0 && cfg_git == 1) {
		add_branch(pwd, VCS_GIT);
	}

	wcstombs(mbpwd, pwd, MAXPATHLEN);
	puts(mbpwd);
}


#ifndef TESTING
int
main(int argc, char **argv)
{
	char *t;
	int opt, run_dump_alias_vars = 0;

	while ((opt = getopt(argc, argv, "aVh")) != -1) {
		switch (opt) {
		case 'a':
			run_dump_alias_vars = 1;
			break;
		case 'V':
			show_version();
			exit(-1);
		default:
			printf("usage: prwd [-aVh]\n");
			exit(-1);
		}
	}

	setlocale(LC_ALL, "");

	/* Populate $HOME */
	t = getenv("HOME");
	mbstowcs(home, t, MAXPATHLEN);
	if (home == NULL || *home == '\0')
		errx(0, "Unknown variable '$HOME'.");

	read_config();

	if (run_dump_alias_vars) {
		dump_alias_vars();
	} else {
		prwd();
	}

	return 0;
}
#endif
