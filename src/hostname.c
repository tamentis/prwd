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

#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "hostname.h"
#include "utils.h"
#include "wgetopt.h"

#define ERR_BAD_ARG L"<hostname-bad-arg>"
#define ERR_BAD_CHARSET L"<hostname-bad-charset>"
#define ERR_GENERIC L"<hostname-error>"

/*
 * This module should never crash and will always return a value on *out.  If
 * any error occur during its runtime, it should be represented in a user
 * readable format on *out.
 */
void
hostname_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	int longform = 0;
	wchar_t ch;
	char buf[MAXHOSTNAMELEN], *c;

	if (lgethostname(buf, MAXHOSTNAMELEN) != 0) {
		wcslcpy(out, ERR_GENERIC, len);
		return;
	}

	woptreset = 1;
	woptind = 0;
	wopterr = 0;
	while ((ch = wgetopt(argc, argv, L"l")) != -1) {
		switch (ch) {
		case L'l':
			longform = 1;
			break;
		default:
			wcslcpy(out, ERR_BAD_ARG, len);
			return;
		}
	}

	/* Find the first dot and stop right here for the short hostname.. */
	if (!longform && (c = strchr(buf, '.')) != NULL)
		*c = '\0';

	if (mbstowcs(out, buf, len) == (size_t)-1)
		wcslcpy(out, ERR_BAD_CHARSET, len);
}
