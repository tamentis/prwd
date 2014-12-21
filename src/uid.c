/*
 * Copyright (c) 2014 Bertrand Janin <b@janin.com>
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

#include <unistd.h>
#include <wchar.h>
#include <err.h>

#include "prwd.h"
#include "uid.h"

/*
 * Add the UID indicator to the given path.  For example "/etc" turns into
 * "/etc$" if your user is non-root and "/etc#" if she/he is root.
 */
void
add_uid_indicator(wchar_t *path)
{
	wchar_t buf[MAX_OUTPUT_LEN];
	wchar_t c;

	if (getuid() == 0) {
		c = L'#';
	} else {
		c = L'$';
	}

	if (swprintf(buf, MAX_OUTPUT_LEN, L"%ls%lc", path, c) == -1)
		errx(1, "failed to add uid indicator");

	wcslcpy(path, buf, MAX_OUTPUT_LEN);
}
