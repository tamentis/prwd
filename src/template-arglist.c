/*
 * Copyright (c) 2014-2015 Bertrand Janin <b@janin.com>
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

/*
 * An arglist allows you to store argv-style data in an automatic variable.
 * All the arguments are stored in a single wchar_t array delimited by
 * NUL-bytes.  The addresses of the arguments are stored in the argv pointer.
 */

#include <wchar.h>
#include <string.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "template.h"

/*
 * Initialize the provided arglist.  Basically set the count (argc) to zero.
 */
void
template_arglist_init(struct arglist *al)
{
	al->argc = 0;
	al->len = 0;
}

/*
 * Add a single element to an arglist.
 */
size_t
template_arglist_insert(struct arglist *al, wchar_t *arg)
{
	size_t l, max;

	max = MAX_ARGLIST_SIZE - al->len;

	if (al->argc + 1 > MAX_ARG_COUNT)
		return (size_t)-1;

	l = wcslcpy(al->value + al->len, arg, max);
	if (l > max) {
		return (size_t)-1;
	}

	al->argv[al->argc] = al->value + al->len;
	al->len += l + 1;
	al->argc++;

	return (al->argc);
}
