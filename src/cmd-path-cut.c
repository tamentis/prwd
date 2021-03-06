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
#include <err.h>

#include "cmd-path.h"
#include "prwd.h"
#include "wcslcpy.h"

#define ERR_NULL_PATH L"<path-null>"

/*
 * Reduce the provided string to the smallest it could get to fit within the
 * global max length and without cutting any path element. For example
 * "/usr/local/share/doc" is reduced to ".../share/doc".
 */
void
path_cleancut(wchar_t *out, wchar_t *path, size_t len, size_t maxlen,
    wchar_t *filler)
{
	size_t maxplen, flen;
	wchar_t *c;

	if (len == 0 || out == NULL)
		errx(1, "path_cleancut: invalid output");
	if (path == NULL)
		errx(1, "path_cleancut: null input");

	/* Normalize len instead dealing with two limits. */
	if (maxlen + 1 > len)
		errx(1, "path_cleancut: maxlen too long");
	len = maxlen + 1;

	/* Path is already short enough. */
	if (wcslen(path) <= maxlen) {
		wcslcpy(out, path, len);
		return;
	}

	/* Copy the filler, everyone needs that. */
	flen = wcslcpy(out, filler, len);

	/* We already reached the max, leave here, there is nothing to add. */
	if (flen >= maxlen)
		return;

	maxplen = maxlen - flen;

	/* Keep triming until 'path' is smaller than maxplen. */
	for (c = path; wcslen(c) > maxplen;) {
		if (*c == L'/')
			c++;
		c = wcschr(c, '/');
		/*
		 * If we reached the end of the string before we truncated the
		 * path enough, just truncate it.
		 */
		if (c == NULL) {
			path_quickcut(out, path, len, maxlen, filler);
			return;
		}
	}

	wcsncpy(out + flen, c, len - flen);
}

/*
 * Reduce the given string with the global max length and filler.  Given a
 * maxlen of 7, filler of ".." and an input of "/usr/local/share/doc",
 * the path would be transformed to "..e/doc"
 */
void
path_quickcut(wchar_t *out, wchar_t *path, size_t len, size_t maxlen,
    wchar_t *filler)
{
	size_t plen, offset, excess;

	if (len == 0 || out == NULL)
		errx(1, "path_quickcut: invalid output");
	if (path == NULL)
		errx(1, "path_quickcut: null input");

	/* Normalize len instead dealing with two limits. */
	if (maxlen + 1 > len)
		errx(1, "path_quickcut: maxlen too long");
	len = maxlen + 1;

	plen = wcslen(path);
	if (plen <= maxlen) {
		wcslcpy(out, path, len);
		return;
	}

	/* Copy the filler, everyone needs that. */
	offset = wcslcpy(out, filler, len);

	/* We already reached the max, leave here, there is nothing to add. */
	if (offset >= maxlen)
		return;

	excess = plen - maxlen;
	path += excess + offset;

	wcslcpy(out + offset, path, len - offset);
}
