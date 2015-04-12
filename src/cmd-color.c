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

#include "cmd-color.h"
#include "strlcpy.h"
#include "utils.h"
#include "wcslcpy.h"
#include "wcstonum.h"

#define ERR_BAD_ARG L"<color-bad-arg>"
#define ERR_BAD_CODE L"<color-bad-code>"

/*
 * This module should never crash and will always return a value on *out.  If
 * any error occur during its runtime, it should be represented in a user
 * readable format on *out.
 */
void
cmd_color_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	(void)argc;
	(void)argv;
	long long code;
	const wchar_t *errstr = NULL;

	if (argc != 2) {
		wcslcpy(out, ERR_BAD_ARG, len);
		return;
	}

	if (wcscmp(argv[1], L"reset") == 0) {
		wcslcpy(out, L"[0;0m", len);
		return;
	}

	code = wcstonum(argv[1], 0, 255, &errstr);
	if (errstr != NULL) {
		wcslcpy(out, ERR_BAD_CODE, len);
		return;
	}

	swprintf(out, len, L"[38;5;%dm", code);
}
