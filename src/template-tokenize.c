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

#include "wcslcpy.h"
#include "template.h"

enum fsm_state {
	STATE_DOLLAR,
	STATE_STATIC,
	STATE_STATIC_END,
	STATE_DYNAMIC,
	STATE_DYNAMIC_END,
	STATE_APPEND_TOKEN
};

#define ERRSTR_TOKEN_SIZE L"invalid token size"
#define ERRSTR_TOO_MANY L"too many tokens"

/*
 * Given a template wide-char string 's', split all the tokens within and set
 * them on the tokens string array, the size of the array is passed as 'len'.
 * The number of tokens saved on 'tokens' is returned. In case of error, -1 is
 * returned.
 *
 * For example, given the following string "this is ${a var} with ${things}",
 * the 'tokens' array would contain the following strings:
 *
 *     - this is	(TOKEN_STATIC)
 *     - a var		(TOKEN_DYNAMIC)
 *     - with		(TOKEN_STATIC)
 *     - things		(TOKEN_DYNAMIC)
 *
 * The return value would be 4.
 */
int
template_tokenize(wchar_t *s, struct token *tokens, size_t len,
    const wchar_t **errstrp)
{
	enum fsm_state state, next_state;
	wchar_t buf[MAX_TOKEN_LEN];
	size_t cur;
	int count = 0;

	*errstrp = NULL;
	state = next_state = STATE_STATIC;
	cur = 0;
	for (;;) {
		switch (state) {
		case STATE_STATIC:
			if (*s == L'\0') {
				state = STATE_STATIC_END;
				break;
			}
			if (*s == L'$') {
				state = STATE_DOLLAR;
				s++;
				break;
			}
			buf[cur++] = *(s++);
			break;
		case STATE_STATIC_END:
			tokens[count].type = TOKEN_STATIC;
			state = STATE_APPEND_TOKEN;
			next_state = STATE_DYNAMIC;
			break;
		case STATE_DOLLAR:
			if (*s == L'{') {
				state = STATE_STATIC_END;
				break;
			}
			buf[cur++] = L'$';
			state = STATE_STATIC;
			break;
		case STATE_DYNAMIC:
			if (*s == L'\0') {
				state = STATE_DYNAMIC_END;
				break;
			}
			if (*s == L'}') {
				state = STATE_DYNAMIC_END;
				break;
			}
			buf[cur++] = *(s++);
			break;
		case STATE_DYNAMIC_END:
			tokens[count].type = TOKEN_DYNAMIC;
			state = STATE_APPEND_TOKEN;
			next_state = STATE_STATIC;
			break;
		case STATE_APPEND_TOKEN:
			/* Copy the new token on the array */
			if (cur > 0) {
				buf[cur] = L'\0';
				if (wcslcpy(tokens[count].value, buf,
				    MAX_TOKEN_LEN) > MAX_TOKEN_LEN) {
					*errstrp = ERRSTR_TOKEN_SIZE;
					return (-1);
				}
				count++;
				if ((size_t)count >= len) {
					*errstrp = ERRSTR_TOO_MANY;
					return (-1);
				}
				cur = 0;
			}

			if (*s == L'\0')
				goto done;

			state = next_state;
			s++;
			break;
		default:
			break;
		}

		if (cur >= MAX_TOKEN_LEN) {
			*errstrp = ERRSTR_TOKEN_SIZE;
			return (-1);
		}
	}

done:
	return (count);
}
