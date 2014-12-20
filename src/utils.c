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

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

/*
 * Panic exit, preferably screaming, running into walls with your arms in the
 * air.
 */
#ifndef TESTING
/*
 * Check if a file exists (works fine for directories too).
 */
int
file_exists(char *filepath)
{
	struct stat sb;

	if (stat(filepath, &sb) != 0) {
		if (errno == ENOENT) {
			return 0;
		}
		errx(1, "stat() failed on %s", filepath);
	}

	return (1);
}

#else	// ifndef TESTING
int file_exists(char *);
#endif	// ifndef TESTING

int
wc_file_exists(wchar_t *wc_filepath)
{
	char mb_filepath[MAXPATHLEN];

	wcstombs(mb_filepath, wc_filepath, MAXPATHLEN);

	return (file_exists(mb_filepath));
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
