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
#include "aliases.h"


extern int cfg_cleancut;
extern int cfg_maxpwdlen;
extern int cfg_mercurial;
extern int cfg_git;
extern int cfg_hostname;
extern int cfg_uid_indicator;
extern int cfg_newsgroup;
extern wchar_t cfg_filler[FILLER_LEN];

extern int alias_count;
extern struct alias_t aliases[MAX_ALIASES];

wchar_t	 home[MAXPATHLEN];


/*
 * Replace all the words by their first letters, except the last one.
 *
 * Input:  /usr/local/share/doc
 * Output: /u/l/s/doc
 */
void
newsgroupize(wchar_t *s)
{
	wchar_t buffer[MAX_OUTPUT_LEN];
	wchar_t *last = NULL, *org = s;
	int idx = 0;

	/* Already as short as we can get it. */
	if (s == NULL || wcslen(s) < 3)
		return;

	/*
	 * Since we join first letters with slashes by the end of this
	 * function, make sure we keep the first character if it isn't a slash.
	 */
	if (*s != L'/')
		buffer[idx++] = *s;

	/* Keep the first part if it's an alias (start by * or $). */
	if (*s == L'*' || *s == L'$') {
		wchar_t *sl = wcschr(s, L'/');

		if (sl) {
			wcslcpy(buffer, s, sl - s + 1);
			idx += sl - s - 1;
			s = sl;
			/*
			 * If we have no other /, keep the alias AND the last
			 * part, so we return without doing anything.
			 */
			sl = wcschr(s + 1, L'/');
			if (sl == NULL) {
				return;
			}
		}
	}
	buffer[idx++] = '/';
	buffer[idx] = L'\0';

	/* For every component, add the first letter and a slash. */
	while ((s = wcschr(s, L'/')) != NULL) {
		/* Cater for trailing slashes. */
		if (s[1] == L'\0')
			break;
		last = ++s;
		buffer[idx++] = (wchar_t)*s;
		buffer[idx++] = L'/';
	}

	/* idx is less than 4, we only have one slash, just keep org as is */
	if (idx < 4)
		return;

	/* Copy letters+slash making sure the last part is left untouched. */
	wcslcpy(org, buffer, idx);
	if (last != NULL)
		wcslcpy(org + idx - 2, last, wcslen(last) + 1);
}


/*
 * Reduce the given string with the global max length and filler.
 *
 * Input:  /usr/local/share/doc
 * Output: ...are/doc
 */
void
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
size_t
get_mercurial_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c;
	char pwd[MAX_OUTPUT_LEN];
	char candidate[MAXPATHLEN];
	char buf[MAX_BRANCH_LEN];
	size_t branch_size;
	struct stat bufstat;
	int found_repo = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

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


/*
 * Recurse up from $PWD to find a .git/ directory with a valid HEAD file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
size_t
get_git_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c;
	char pwd[MAX_OUTPUT_LEN];
	char candidate[MAXPATHLEN];
	char buf[MAX_BRANCH_LEN];
	size_t s;
	struct stat bufstat;
	int found_repo = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

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

	/* That's probably just a changeset, just show the first 6 chars. */
	if (s > 6) {
		strlcpy(buf + 6, "...", 4);
		return mbstowcs(dst, buf, MAX_BRANCH_LEN);
	}

	/* We shouldn't get there, but we mind as well no crash. */
	strlcpy(buf, "???", 4);
	return mbstowcs(dst, buf, MAX_BRANCH_LEN);
}


/*
 * Add the mercurial branch at the beginning of the path. If a branch was
 * found, 1 is returned else 0.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
int
add_branch(wchar_t *s, enum version_control_system vcs)
{
	wchar_t org[MAX_OUTPUT_LEN];
	wchar_t branch[MAX_BRANCH_LEN];
	size_t len;

	wcslcpy(org, s, MAX_OUTPUT_LEN);

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
	wcslcpy(s, org, MAX_OUTPUT_LEN - len - 1);

	return 1;
}


/*
 * This wrapper around gethostname is overwritten by the test suite.
 */
#ifndef TESTING
int
get_full_hostname(char *buf, size_t size)
{
	return gethostname(buf, size);
}
#endif


/*
 * Add the hostname in front of the path.
 *
 * Input:  /etc
 * Output: odin:/etc
 */
void
add_hostname(wchar_t *s)
{
	char buf[MAXHOSTNAMELEN], *c;
	wchar_t org[MAX_OUTPUT_LEN];
	wchar_t hostname[MAX_HOSTNAME_LEN];
	size_t len;

	wcslcpy(org, s, MAX_OUTPUT_LEN);

	/* We failed to get the hostname. Complain and die. */
	if (get_full_hostname(buf, MAXHOSTNAMELEN) != 0)
		return fatal("prwd: gethostname() failed");

	/* Find the first dot and stop right here. */
	c = strchr(buf, '.');
	if (c != NULL)
		*c = '\0';

	if (mbstowcs(hostname, buf, MAX_HOSTNAME_LEN) == -1)
		return fatal("prwd: mbstowcs(hostname, ...) failed");

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
void
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
void
cleancut(wchar_t *s)
{
	int flen;
	wchar_t *last = NULL, t[MAX_OUTPUT_LEN], *org = s;

	/* NULL or empty input, nothing to touch */
	if (s == NULL || *s == L'\0')
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
	wcslcpy(t, s, MAX_OUTPUT_LEN);
	wcslcpy(org, t, MAX_OUTPUT_LEN);
}


/*
 * Loop through the user-defined aliases and find the best match to
 * get the shortest path as possible.
 */
void
replace_aliases(wchar_t *s)
{
	int i, chosen = -1;
	size_t len, nlen, max = 0;
	wchar_t t[MAX_OUTPUT_LEN], *org = s;

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

	wcslcpy(t, s, MAX_OUTPUT_LEN);
	wcslcpy(org, t, MAX_OUTPUT_LEN);
}


void
show_version(void)
{
	puts("prwd-" PRWD_VERSION);
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
	wchar_t path[MAX_OUTPUT_LEN];

	for (i = 0; i < alias_count; i++) {
		if (aliases[i].name[0] == '$') {
			wcslcpy(path, aliases[i].path, MAX_OUTPUT_LEN);
			expand_prefix_aliases(path, MAX_OUTPUT_LEN);
			if (!wc_file_exists(path))
				continue;
			wprintf(L"export %ls=\"%ls\"\n", aliases[i].name + 1,
					path);
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
	char *t = NULL;
	char mbpwd[MAX_OUTPUT_LEN];
	wchar_t pwd[MAX_OUTPUT_LEN];

	/*
	 * Attempt to read the shell's PWD environment variable to obtain the
	 * current directory. If this fails (shell has no such variable) or if
	 * it was configured otherwise, use the return from getcwd().
	 */
	t = getenv("PWD");
	if (t == NULL || t[0] == '\0')
		t = getcwd(NULL, MAXPATHLEN);
	if (t == NULL)
		errx(0, "Unable to get current working directory.");

	mbstowcs(pwd, t, MAX_OUTPUT_LEN);

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

	/* If mercurial or git is enabled, show the branch */
	if (cfg_mercurial == 1) {
		found_repo = add_branch(pwd, VCS_MERCURIAL);
	}

	if (found_repo == 0 && cfg_git == 1) {
		add_branch(pwd, VCS_GIT);
	}

	/* Do we show the hostname? */
	if (cfg_hostname == 1) {
		add_hostname(pwd);
	}

	/* Add the '$' or '#' character depending if your root. */
	if (cfg_uid_indicator == 1) {
		add_uid_indicator(pwd);
	}

	wcstombs(mbpwd, pwd, MAX_OUTPUT_LEN);
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
	if (home == NULL || *home == L'\0')
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
