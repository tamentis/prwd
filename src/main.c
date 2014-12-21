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

#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <err.h>
#include <locale.h>

#include "prwd.h"
#include "config.h"
#include "alias.h"
#include "uid.h"
#include "cut.h"
#include "newsgroupize.h"
#include "vcs.h"
#include "hostname.h"

extern int cfg_cleancut;
extern size_t cfg_maxpwdlen;
extern int cfg_mercurial;
extern int cfg_git;
extern int cfg_hostname;
extern int cfg_uid_indicator;
extern int cfg_newsgroup;

wchar_t	 home[MAXPATHLEN];


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
	alias_replace(wcs_wd);

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

#ifndef REGRESS
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
#endif	// ifndef REGRESS
