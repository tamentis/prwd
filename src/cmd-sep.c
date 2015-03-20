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

#include <wchar.h>

#include "cmd-sep.h"

#define ERR_BAD_ARG L"<sep-bad-arg>"

/*
 * This module should never crash and will always return a value on *out.  If
 * any error occur during its runtime, it should be represented in a user
 * readable format on *out.
 */
void
cmd_sep_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	(void)argc;
	(void)argv;

	if (argc != 2) {
		wcslcpy(out, ERR_BAD_ARG, len);
		return;
	}

	swprintf(out, len, L"%ls", argv[1]);
}
