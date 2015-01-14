/*
 * Copyright (c) 2015 Bertrand Janin <b@janin.com>
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
#include <sys/param.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wchar.h>

#include "prwd.h"
#include "strlcpy.h"
#include "wcslcpy.h"
#include "wgetopt.h"
#include "path.h"

#define ERR_NO_ACCESS L"<path-no-access>"
#define ERR_NOT_FOUND L"<path-not-found>"
#define ERR_BAD_CHARSET L"<path-bad-charset>"
#define ERR_BAD_ARG L"<path-bad-arg>"
#define ERR_GENERIC L"<path-bad-arg>"

/*
 * Return a wide-char version of the current path.  If any error occurs,
 * *errstr is set to a replacement string to be used instead of the path.
 */
#ifndef REGRESS
void
path_wcswd(wchar_t *wcswd, size_t len, const wchar_t **errstr)
{
	char wd[MAXPATHLEN], *wd_env;
	struct stat sa, sb;

	*errstr = NULL;

	/*
	 * On most platform getcwd() returns NULL if the directory is missing.
	 * This could happen if the directory you are currently in was removed.
	 * If this occurs, we don't need to go any further, we have nothing
	 * better to display.
	 */
	if (getcwd(wd, MAXPATHLEN) == NULL) {
		switch (errno) {
		case EACCES:
			*errstr = ERR_NO_ACCESS;
			break;
		case ENOENT:
			*errstr = ERR_NOT_FOUND;
			break;
		default:
			*errstr = ERR_GENERIC;
			break;
		}
		return;
	}

	if (stat(wd, &sa) == -1) {
		*errstr = ERR_GENERIC;
		return;
	}

	/*
	 * If we can get a valid PWD from the environment, that turns out to be
	 * the same directory, then we should use it, it provides more context
	 * if the shell is located in a symlink.
	 */
	wd_env = getenv("PWD");
	if (wd_env != NULL && stat(wd_env, &sb) == 0) {
		if (sa.st_ino == sb.st_ino && sa.st_dev == sb.st_dev) {
			strlcpy(wd, wd_env, MAXPATHLEN);
		}
	}

	if (mbstowcs(wcswd, wd, len) == (size_t)-1) {
		*errstr = ERR_BAD_CHARSET;
		return;
	}
}
#endif /* ifndef REGRESS */

/*
 * This module should never crash and will always return a value on out.  If
 * any error occur during its runtime, it will return an error code to help the
 * user to understand what's going on (instead of a fatal error which trashes
 * the prompt output).
 */
void
path_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	int newsgroupize = 0;
	const wchar_t *errstr = NULL;
	wchar_t ch, wcswd[MAXPATHLEN];

	path_wcswd(wcswd, MAXPATHLEN, &errstr);
	if (errstr != NULL) {
		wcslcpy(out, errstr, len);
		return;
	}

	woptreset = 1;
	woptind = 0;
	while ((ch = wgetopt(argc, argv, L"n")) != -1) {
		switch (ch) {
		case L'n':
			newsgroupize = 1;
			break;
		default:
			wcslcpy(out, ERR_BAD_ARG, len);
			return;
		}
	}

	if (newsgroupize) {
		path_newsgroupize(out, wcswd, len);
	} else {
		wcslcpy(out, wcswd, len);
	}
}
