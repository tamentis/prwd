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
#include <wctype.h>
#include <string.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "template.h"
#include "utils.h"

enum fsm_state {
	STATE_ARG,
	STATE_ARG_END,
	STATE_SPACE,
	STATE_QUOTED,
	STATE_QUOTED_BACKSLASH,
	STATE_BACKSLASH,
	STATE_ESCAPED
};

#define ERRSTR_TOO_LARGE L"argument list too large"
#define ERRSTR_UNMATCHED_QUOTE L"unmatched quote"

/*
 * Run a lexical analysis on a variable token extracted from a template.  This
 * lexer is particularly simple and only splits using spaces and is able to
 * work around quoted arguments and some escaped values.
 *
 * For example, given the following wide-char string:
 *
 *     path -nt foo "bar baz"
 *
 * This lexer would return the following arguments:
 *
 *     * path
 *     * -nt
 *     * foo
 *     * bar baz
 *
 * The return value would be 4.  The result is written on the provided arglist,
 * which could be passed to getopt() with its argc and argv properties.
 */
size_t
template_variable_lexer(wchar_t *s, struct arglist *al,
    const wchar_t **errstrp)
{
	enum fsm_state state, next_state;
	wchar_t buf[MAX_ARGLIST_SIZE];
	size_t cur;

	*errstrp = NULL;
	state = next_state = STATE_ARG;
	cur = 0;
	for (;;) {
		switch (state) {
		case STATE_ARG:
			if (*s == L'\0') {
				state = STATE_ARG_END;
				break;
			}
			if (*s == L'\\') {
				state = STATE_BACKSLASH;
				s++;
				break;
			}
			if (*s == L'"') {
				state = STATE_QUOTED;
				s++;
				break;
			}
			if (iswspace(*s)) {
				state = STATE_ARG_END;
				next_state = STATE_SPACE;
				break;
			}
			buf[cur++] = *(s++);
			break;
		case STATE_SPACE:
			if (*s == L'\0')
				goto done;
			if (iswspace(*s)) {
				s++;
				break;
			}
			if (*s == L'"') {
				state = STATE_QUOTED;
				s++;
				break;
			}
			state = STATE_ARG;
			break;
		case STATE_BACKSLASH:
			buf[cur++] = *(s++);
			state = STATE_ARG;
			break;
		case STATE_ARG_END:
			/* Copy the new arg on the array */
			if (cur > 0) {
				buf[cur] = L'\0';
				if (template_arglist_insert(al, buf) ==
				    (size_t)-1) {
					*errstrp = ERRSTR_TOO_LARGE;
					return ((size_t)-1);
				}
				cur = 0;
			}

			if (*s == L'\0')
				goto done;

			state = next_state;
			s++;
			break;
		case STATE_QUOTED:
			if (*s == L'\\') {
				state = STATE_QUOTED_BACKSLASH;
				s++;
				break;
			}
			if (*s == L'"') {
				state = STATE_ARG;
				s++;
				break;
			}
			if (*s == L'\0') {
				*errstrp = ERRSTR_UNMATCHED_QUOTE;
				return ((size_t)-1);
			}
			buf[cur++] = *(s++);
			break;
		case STATE_QUOTED_BACKSLASH:
			buf[cur++] = *(s++);
			state = STATE_QUOTED;
			break;
		default:
			break;
		}

		if (cur >= sizeof(buf) / sizeof(wchar_t)) {
			*errstrp = ERRSTR_TOO_LARGE;
			return ((size_t)-1);
		}
	}

done:
	return (al->argc);
}
