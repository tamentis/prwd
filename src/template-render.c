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

#include <wchar.h>
#include <string.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "template.h"

#define ERRSTR_OUTPUT_SIZE L"output buffer too short for rendered template"

/*
 * Execute the provided template 'tmpl' and save the output to 'output'.  In
 * case of error, return -1 and set errstrp to an error message.
 */
int
template_render(wchar_t *tmpl, wchar_t *out, size_t len,
    const wchar_t **errstrp)
{
	struct token tokens[MAX_TOKEN_COUNT];
	wchar_t buf[MAX_OUTPUT_LEN], *c;
	int i, count, prevempty;
	size_t cur, tlen;

	count = template_tokenize(tmpl, tokens, MAX_TOKEN_COUNT, errstrp);
	if (count == -1) {
		return (-1);
	}

	cur = 0;
	prevempty = 0;
	for (i = 0; i < count; i++) {
		if (tokens[i].type == TOKEN_STATIC) {
			c = tokens[i].value;
			prevempty = 0;
		} else {
			tlen = template_exec_cmd(tokens[i].value, buf,
			    MAX_OUTPUT_LEN, prevempty, errstrp);
			if (*errstrp != NULL)
				return (-1);
			if (tlen == 0) {
				prevempty = 1;
			} else {
				prevempty = 0;
			}
			c = buf;
		}

		tlen = wcslcpy(out + cur, c, len - cur);
		if (tlen > len - cur) {
			*errstrp = ERRSTR_OUTPUT_SIZE;
			return (-1);
		}
		cur += tlen;
	}

	out[cur] = L'\0';

	return (0);
}
