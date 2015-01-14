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
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

/*
 * Check if a file exists (works fine for directories too).
 */
#ifndef REGRESS
int
path_is_valid(char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		if (errno == ENOENT) {
			return (0);
		}
		err(1, "stat() failed on %s", path);
	}

	return (1);
}
#endif	// ifndef REGRESS

/*
 * Checks if a file exists given a wchar path.
 */
int
wc_path_is_valid(wchar_t *wpath)
{
	char path[MAXPATHLEN];
	wcstombs(path, wpath, MAXPATHLEN);
	return (path_is_valid(path));
}

/*
 * Check if a path is valid using a format string.
 */
int
fmt_path_is_valid(char *fmt, ...)
{
	int valid;
	va_list ap;
	char path[MAXPATHLEN];

	va_start(ap, fmt);
	vsnprintf(path, MAXPATHLEN, fmt, ap);
	valid = path_is_valid(path);
	va_end(ap);

	return (valid);
}

/*
 * Copy all the characters from input to token until we find a '/' or NUL-byte.
 */
void
tokcpy(wchar_t *input, wchar_t *token)
{
	int i;

	for (i = 0; input[i] != '/' && input[i] != '\0'; i++) {
		token[i] = input[i];
	}

	token[i] = '\0';
}

/*
 * Overridable gethostname().
 */
#ifndef REGRESS
int
lgethostname(char *buf, size_t size)
{
	return (gethostname(buf, size));
}
#endif

/*
 * Check if a wide-char is a space (same definition as isspace).
 */
int
iswspace(wchar_t c)
{
	if (wcschr(L" \f\n\r\t\v", c) == NULL) {
		return (0);
	} else {
		return (1);
	}
}
