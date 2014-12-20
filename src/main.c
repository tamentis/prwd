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
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <err.h>
#include <locale.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "strlcpy.h"
#include "config.h"
#include "utils.h"
#include "alias.h"

extern int cfg_cleancut;
extern size_t cfg_maxpwdlen;
extern int cfg_mercurial;
extern int cfg_git;
extern int cfg_hostname;
extern int cfg_uid_indicator;
extern int cfg_newsgroup;
extern wchar_t cfg_filler[FILLER_LEN];

// extern int alias_count;
extern struct alias aliases[MAX_ALIASES];

wchar_t	 home[MAXPATHLEN];

/*
 * Replace all the words by their first letters, except the last one.
 *
 * Input:  /usr/local/share/doc
 * Output: /u/l/s/doc
 */
static void
newsgroupize(wchar_t *s)
{
	wchar_t buf[MAX_OUTPUT_LEN];
	wchar_t *last = NULL, *org = s;
	int idx = 0;

	/* Already as short as we can get it. */
	if (s == NULL || wcslen(s) < 3)
		return;

	/*
	 * The path doesn't start with a '/', could be an alias, could be '~'.
	 * Copy everything until the first slash.
	 */
	if (*s != L'/') {
		do {
			buf[idx++] = *(s++);
		} while (*s != L'/' && *s != L'\0');
	}

	/* We already reached the end, that means the string was fine as-is. */
	if (*s == L'\0')
		return;

	/* For every component, add the first letter and a slash. */
	for (;;) {
		/* Copy the slash and move on. */
		buf[idx++] = *(s++);
		last = s;

		/* Is there more to come? */
		if ((s = wcschr(s, L'/')) == NULL)
			break;

		/* Trailing slash? */
		if (*(s + 1) == L'\0')
			break;

		buf[idx++] = (wchar_t)*last;
	}

	/* Copy whatever is left (override the trailing NUL-byte on buffer) */
	wcslcpy(buf + idx, last, sizeof(buf) - idx);

	/* Copy letters+slash making sure the last part is left untouched. */
	wcslcpy(org, buf, sizeof(buf));
}


/*
 * Reduce the given string with the global max length and filler.
 *
 * Input:  /usr/local/share/doc
 * Output: ...are/doc
 */
static void
quickcut(wchar_t *s, size_t len)
{
	wchar_t t[MAX_OUTPUT_LEN];
	size_t	filler_len = wcslen(cfg_filler), cl = sizeof(wchar_t);

	if (s == NULL || len == 0 || *s == L'\0' || len <= cfg_maxpwdlen)
		return;

	wcslcpy(t, cfg_filler, filler_len + cl);
	wcslcpy(t + filler_len, s + len - cfg_maxpwdlen + filler_len,
			cfg_maxpwdlen - filler_len + cl);
	wcslcpy(s, t, cfg_maxpwdlen + cl);
}


/*
 * Recurse up from $PWD to find a .hg/ directory with a valid branch file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
static size_t
get_mercurial_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c, pwd[MAX_OUTPUT_LEN], path[MAXPATHLEN], buf[MAX_BRANCH_LEN];
	size_t branch_size;
	struct stat bufstat;
	int found = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

	do {
		snprintf(path, MAXPATHLEN, "%s/.hg/branch", pwd);

		found = stat(path, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found != 0 && path[1] != '\0');

	if (found == -1)
		return (0);

	fp = fopen(path, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		return (mbstowcs(dst, buf, MAX_BRANCH_LEN));
	}

	if (fread(buf, 1, size, fp) == 0)
		err(1, "failed to read the .hg/branch file");
	fclose(fp);

	/* remove the trailing new line if any */
	if ((c = strchr(buf, '\n')) != NULL)
		*c = '\0';

	branch_size = mbstowcs(dst, buf, MAX_BRANCH_LEN);

	return (branch_size);
}

/*
 * Recurse up from $PWD to find a .git/ directory with a valid HEAD file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
static size_t
get_git_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c, path[MAXPATHLEN], buf[MAX_BRANCH_LEN], pwd[MAX_OUTPUT_LEN];
	size_t s;
	struct stat bufstat;
	int found = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

	do {
		snprintf(path, MAXPATHLEN, "%s/.git/HEAD", pwd);

		found = stat(path, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found != 0 && path[1] != '\0');

	if (found == -1)
		return (0);

	fp = fopen(path, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		c = buf;
		goto finish;
	}

	s = fread(buf, 1, size, fp);
	fclose(fp);

	buf[MAX_BRANCH_LEN - 1] = '\0';

	/* This is a branch head, just print the branch. */
	if (strncmp(buf, "ref: refs/heads/", 16) == 0) {
		c = strchr(buf, '\n');
		if (c)
			*(c) = '\0';
		c = buf + 16;
		goto finish;
	}

	/* Show all other kinds of ref as-is (does it even exist?) */
	if (strncmp(buf, "ref:", 4) == 0) {
		c = strchr(buf, '\n');
		if (c)
			*(c) = '\0';
		c = buf + 5;
		goto finish;
	}

	/* That's probably just a changeset, just show the first 6 chars. */
	if (s > 6) {
		strlcpy(buf + 6, "...", 4);
		c = buf;
		goto finish;
	}

	/* We shouldn't get there, but we mind as well no crash. */
	strlcpy(buf, "???", 4);
	c = buf;

finish:
	return (mbstowcs(dst, c, MAX_BRANCH_LEN));
}

/*
 * Add the mercurial branch at the beginning of the path. If a branch was
 * found, 1 is returned else 0.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
static int
add_branch(wchar_t *s, enum vcs_types vcs)
{
	wchar_t org[MAX_OUTPUT_LEN];
	wchar_t branch[MAX_BRANCH_LEN];
	size_t len;

	wcslcpy(org, s, MAX_OUTPUT_LEN);

	switch (vcs) {
		case VCS_MERCURIAL:
			if (get_mercurial_branch(branch, MAX_BRANCH_LEN) == 0)
				return (0);
			break;
		case VCS_GIT:
			if (get_git_branch(branch, MAX_BRANCH_LEN) == 0)
				return (0);
			break;
		default:
			return (0);
	}

	len = wcslcpy(s, branch, MAX_BRANCH_LEN);

	s += len;
	*(s++) = ':';
	wcslcpy(s, org, MAX_OUTPUT_LEN - len - 1);

	return (1);
}

/*
 * This wrapper around gethostname is overwritten by the test suite.
 */
#ifdef TESTING
int get_full_hostname(char *, size_t);
#else
static int
get_full_hostname(char *buf, size_t size)
{
	return (gethostname(buf, size));
}
#endif

/*
 * Add the hostname in front of the path.
 *
 * Input:  /etc
 * Output: odin:/etc
 */
static void
add_hostname(wchar_t *s)
{
	char buf[MAXHOSTNAMELEN], *c;
	wchar_t org[MAX_OUTPUT_LEN];
	wchar_t hostname[MAX_HOSTNAME_LEN];
	size_t len;

	wcslcpy(org, s, MAX_OUTPUT_LEN);

	/* We failed to get the hostname. Complain and die. */
	if (get_full_hostname(buf, MAXHOSTNAMELEN) != 0) {
		errx(1, "gethostname() failed");
	}

	/* Find the first dot and stop right here. */
	c = strchr(buf, '.');
	if (c != NULL)
		*c = '\0';

	if (mbstowcs(hostname, buf, MAX_HOSTNAME_LEN) == (size_t)-1) {
		err(1, "mbstowcs(hostname, ...) failed");
	}

	len = wcslcpy(s, hostname, MAX_HOSTNAME_LEN);

	s += len;
	*(s++) = ':';
	wcslcpy(s, org, MAX_OUTPUT_LEN - len - 1);
}

/*
 * Add the UID indicator.
 *
 * Input:              /etc
 * Output if non-root: /etc$
 * Output if root:     /etc#
 */
static void
add_uid_indicator(wchar_t *s)
{
	wchar_t buf[MAX_OUTPUT_LEN];
	wchar_t c;

	if (getuid() == 0) {
		c = L'#';
	} else {
		c = L'$';
	}

	swprintf(buf, MAX_OUTPUT_LEN, L"%ls%lc", s, c);

	wcslcpy(s, buf, MAX_OUTPUT_LEN);
}

/*
 * Reduce the given string to the smallest it could get to fit within
 * the global max length and without cutting any word.
 *
 * Input:  /usr/local/share/doc
 * Output: .../share/doc
 */
static void
cleancut(wchar_t *s)
{
	size_t flen;
	wchar_t *last = NULL, t[MAX_OUTPUT_LEN], *org = s;

	/* NULL or empty input, nothing to touch */
	if (s == NULL || *s == L'\0')
		return;

	/* Nothing needs to be cropped */
	if (wcslen(s) <= cfg_maxpwdlen)
		return;

	/* As long as we can't fit 's' within the maxpwdlen, keep trimming */
	flen = wcslen(cfg_filler);
	while ((long)wcslen(s) > ((long)cfg_maxpwdlen - (long)flen)) {
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
			goto finish;
		} else {
			s = last;
		}
	}

	s -= flen;
	wcsncpy(s, cfg_filler, flen);

finish:
	wcslcpy(t, s, MAX_OUTPUT_LEN);
	wcslcpy(org, t, MAX_OUTPUT_LEN);
}

/*
 * Loop through the user-defined aliases and find the best match to
 * get the shortest path as possible.
 */
static void
replace_aliases(wchar_t *path)
{
	size_t nlen, plen;
	wchar_t buf[MAX_OUTPUT_LEN];
	struct alias *alias;

	alias = alias_get_by_path(path);
	if (alias == NULL)
		return;

	plen = wcslen(alias->path);
	nlen = wcslcpy(buf, alias->name, MAX_OUTPUT_LEN);
	wcslcpy(buf + nlen, path + plen, MAX_OUTPUT_LEN - nlen);
	wcslcpy(path, buf, MAX_OUTPUT_LEN);
}

/*
 * Main prwd functionality, prints a reduced working directory.
 */
static void
prwd(void)
{
	size_t len;
	int foundvcs = 0;
	const char *errstr;
	char *wd = NULL, *wd_env = NULL, mbs_wd[MAX_OUTPUT_LEN];
	wchar_t wcs_wd[MAX_OUTPUT_LEN];
	struct stat sa, sb;

	wd = getcwd(NULL, MAXPATHLEN);
	if (wd == NULL)
		errx(100, "unable to get current working directory");

	if (stat(wd, &sa) == -1)
		err(100, "stat(wd_real)");

	/*
	 * If we can get a valid PWD from the environment, that turns out to be
	 * the same directory, then we should use it, it provides more context
	 * if the shell is located in a symlink.
	 */
	wd_env = getenv("PWD");
	if (wd_env != NULL && stat(wd_env, &sb) == 0) {
		if (sa.st_ino == sb.st_ino && sa.st_dev == sb.st_dev) {
			free(wd);
			wd = wd_env;
		}
	}

	mbstowcs(wcs_wd, wd, MAX_OUTPUT_LEN);

	/* Replace the beginning with ~ for directories within $HOME. */
	alias_add(L"~", home, &errstr);
	if (errstr != NULL)
		errx(1, "failed to add default \"~\" alias: %s", errstr);

	/* Alias handling */
	replace_aliases(wcs_wd);

	/* Newsgroup mode, keep only the first letters. */
	if (cfg_newsgroup)
		newsgroupize(wcs_wd);

	/* If the path is still too long, crop it. */
	len = wcslen(wcs_wd);

	if (cfg_maxpwdlen > 0 && len > cfg_maxpwdlen) {
		if (cfg_cleancut && !cfg_newsgroup) {
			cleancut(wcs_wd);
		} else {
			quickcut(wcs_wd, len);
		}
	}

	/* If mercurial or git is enabled, show the branch */
	if (cfg_mercurial) {
		foundvcs = add_branch(wcs_wd, VCS_MERCURIAL);
	}

	if (!foundvcs && cfg_git) {
		add_branch(wcs_wd, VCS_GIT);
	}

	/* Do we show the hostname? */
	if (cfg_hostname) {
		add_hostname(wcs_wd);
	}

	/* Add the '$' or '#' character depending if your root. */
	if (cfg_uid_indicator) {
		add_uid_indicator(wcs_wd);
	}

	wcstombs(mbs_wd, wcs_wd, MAX_OUTPUT_LEN);
	puts(mbs_wd);
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
			puts("prwd-"VERSION);
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
	if (home == NULL || *home == L'\0')
		errx(0, "Unknown variable '$HOME'.");

	read_config();

	if (run_dump_alias_vars) {
		alias_dump_vars();
	} else {
		prwd();
	}

	return (0);
}
#endif
