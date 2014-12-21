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

#include "cut.h"
#include "prwd.h"
#include "wcslcpy.h"

extern size_t cfg_maxpwdlen;
extern wchar_t cfg_filler[FILLER_LEN];

/*
 * Reduce the provided string to the smallest it could get to fit within the
 * global max length and without cutting any path element. For example
 * "/usr/local/share/doc" is reduced to ".../share/doc".
 */
void
cleancut(wchar_t *path)
{
	size_t flen;
	wchar_t *last = NULL, t[MAX_OUTPUT_LEN], *org = path;

	/* NULL or empty input, nothing to touch */
	if (path == NULL || *path == L'\0')
		return;

	/* Nothing needs to be cropped */
	if (wcslen(path) <= cfg_maxpwdlen)
		return;

	/* Keep triming until we can fit 'path' within the maxpwdlen */
	flen = wcslen(cfg_filler);
	while ((long)wcslen(path) > ((long)cfg_maxpwdlen - (long)flen)) {
		path++;
		path = wcschr(path, '/');
		if (path == NULL)
			break;
		last = path;
	}

	/* The last element was too long, keep it */
	if (path == NULL) {
		/*
		 * last has never been touched, this means we only have
		 * one slash, revert path to its original value, there is
		 * nothing we can crop.
		 */
		if (last == NULL) {
			path = org;
			goto finish;
		} else {
			path = last;
		}
	}

	path -= flen;
	wcsncpy(path, cfg_filler, flen);

finish:
	wcslcpy(t, path, MAX_OUTPUT_LEN);
	wcslcpy(org, t, MAX_OUTPUT_LEN);
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
