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

#include "path.h"
#include "prwd.h"
#include "template.h"
#include "wcslcpy.h"

#define ERRSTR_EMPTY "empty variable"
#define ERRSTR_UNKCMD "unknown command"
#define ERRSTR_TOO_LARGE "variable output too large"
#define ERRSTR_CMDERR "command error"

/*
 * Execute a single dynamic token.
 *  1. shell tokenize, obtain argc and argv
 *  2. check if we know the command
 *  3. execute the parse_args for this command with the arglist
 *  4. copy the output
 */
int
template_exec(wchar_t *value, wchar_t *out, size_t len, const char **errstrp)
{
	struct arglist al;
	size_t argc;
	wchar_t buf[MAX_OUTPUT_LEN];
	int i = 0;

	template_arglist_init(&al);
	argc = template_variable_lexer(value, &al, errstrp);
	if (argc == (size_t)-1) {
		return (-1);
	}

	if (argc == 0) {
		*errstrp = ERRSTR_EMPTY;
		return (-1);
	}

	if (wcscmp(al.argv[0], L"path") == 0) {
		i = path_exec(argc, al.argv, out, len);
	} else {
		*errstrp = ERRSTR_UNKCMD;
		return (-1);
	}

	if (i == -1) {
		*errstrp = ERRSTR_CMDERR;
		return (-1);
	}

	return (0);
}
