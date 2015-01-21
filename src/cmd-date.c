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

#include <sys/param.h>

#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "cmd-date.h"
#include "strlcpy.h"
#include "utils.h"

#define ERR_BAD_ARG L"<date-bad-arg>"
#define ERR_BAD_CHARSET L"<date-bad-charset>"
#define ERR_BAD_DATE L"<date-bad-format>"
#define ERR_BAD_TIME L"<date-bad-time>"
#define ERR_GENERIC L"<date-error>"

/*
 * This module should never crash and will always return a value on *out.  If
 * any error occur during its runtime, it should be represented in a user
 * readable format on *out.
 */
void
cmd_date_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	(void)argc;
	(void)argv;
	char buf[MAX_DATE_LEN];
	char fmt[MAX_DATE_LEN];
	time_t t;
	struct tm *tm;

	if (argc > 2) {
		wcslcpy(out, ERR_BAD_ARG, len);
		return;
	}

	if (argc == 2) {
		if (wcstombs(fmt, argv[1], MAX_DATE_LEN) == (size_t)-1) {
			wcslcpy(out, ERR_BAD_CHARSET, len);
			return;
		}
	} else {
		strlcpy(fmt, "%H:%M:%S", MAX_DATE_LEN);
	}

	t = time(NULL);
	tm = localtime(&t);
	if (tm == NULL) {
		wcslcpy(out, ERR_BAD_TIME, len);
		return;
	}

	if (strftime(buf, MAX_DATE_LEN, fmt, tm) == 0) {
		wcslcpy(out, ERR_BAD_DATE, len);
		return;
	}

	if (mbstowcs(out, buf, len) == (size_t)-1)
		wcslcpy(out, ERR_BAD_CHARSET, len);
}
