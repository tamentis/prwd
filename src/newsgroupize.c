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

#include <wchar.h>

#include "prwd.h"
#include "newsgroupize.h"
#include "wcslcpy.h"

/*
 * Replace all the path parts by their first letters, except the last one. For
 * example "/usr/local/share/doc" is turned into "/u/l/s/doc".
 */
void
newsgroupize(wchar_t *path)
{
	wchar_t buf[MAX_OUTPUT_LEN], *last = NULL, *c = path;
	int idx = 0;

	/* Already as short as we can get it. */
	if (path == NULL || wcslen(path) < 3)
		return;

	/*
	 * The path doesn't start with a '/', could be an alias, could be '~'.
	 * Copy everything until the first slash.
	 */
	if (*path != L'/') {
		do {
			buf[idx++] = *(c++);
		} while (*c != L'/' && *c != L'\0');
	}

	/* We already reached the end, that means the string was fine as-is. */
	if (*c == L'\0')
		return;

	/* For every component, add the first letter and a slash. */
	for (;;) {
		/* Copy the slash */
		buf[idx++] = *(c++);
		last = c;

		/* Try to move to the next path part */
		if ((c = wcschr(c, L'/')) == NULL)
			break;

		/* This last slash is a trailing one, stop here. */
		if (*(c + 1) == L'\0')
			break;

		/* Copy the character right after the slash. */
		buf[idx++] = *last;
	}

	/* Copy whatever is left (override the trailing NUL-byte on buffer) */
	wcslcpy(buf + idx, last, MAX_OUTPUT_LEN - idx);

	/* Copy letters+slash making sure the last part is left untouched. */
	wcslcpy(path, buf, MAX_OUTPUT_LEN);
}
