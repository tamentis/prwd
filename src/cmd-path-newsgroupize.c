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

#include <wchar.h>

#include "prwd.h"
#include "cmd-path.h"
#include "wcslcpy.h"

/*
 * Replace all the path parts by their first letters, except the last one. For
 * example "/usr/local/share/doc" is turned into "/u/l/s/doc".  Write the
 * result to output.
 */
void
path_newsgroupize(wchar_t *output, const wchar_t *path, size_t len)
{
	const wchar_t *last = NULL, *c = path;
	size_t idx = 0;

	if (len < 1)
		return;
	if (path == NULL)
		return;

	/*
	 * The path doesn't start with a '/', could be an alias, could be '~'.
	 * Copy everything until the first slash.
	 */
	if (*path != L'/') {
		do {
			if (idx >= len) {
				output[idx] = L'\0';
				return;
			}
			output[idx] = *(c++);
			idx++;
		} while (*c != L'/' && *c != L'\0');
	}

	/* Nothing to shorten, we reached the end without finding a '/'. */
	if (*c == L'\0') {
		output[idx] = L'\0';
		return;
	}

	/* For every component, add the first letter and a slash. */
	for (;;) {
		/* Copy the slash and keep the first letter in 'last' */
		output[idx++] = *(c++);
		last = c;

		/* Try to move to the next path part */
		if ((c = wcschr(c, L'/')) == NULL)
			break;

		/* This last slash is a trailing one, stop here. */
		if (*(c + 1) == L'\0')
			break;

		/* Copy the character right after the slash. */
		output[idx++] = *last;
	}

	/* Copy whatever is left (override the trailing NUL-byte on buffer) */
	wcslcpy(output + idx, last, len - idx);
}
