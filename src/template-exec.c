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

#include "cmd-branch.h"
#include "cmd-color.h"
#include "cmd-date.h"
#include "cmd-hostname.h"
#include "cmd-path.h"
#include "cmd-sep.h"
#include "cmd-uid.h"
#include "prwd.h"
#include "template.h"
#include "wcslcpy.h"
#include "wgetopt.h"

#define ERRSTR_EMPTY L"empty variable"
#define ERRSTR_UNKCMD L"unknown command"
#define ERRSTR_TOO_LARGE L"variable output too large"
#define ERRSTR_CMDERR L"command error"

/*
 * Execute a single command.  The prevempty argument defines whether the
 * previous token ended up being empty or not, this is used for the sep
 * command until we find better semantics.
 *
 *  1. shell tokenize, obtain argc and argv
 *  2. check if we know the command
 *  3. execute the parse_args for this command with the arglist
 *  4. copy the output
 */
size_t
template_exec_cmd(wchar_t *value, wchar_t *out, size_t len, int prevempty,
    const wchar_t **errstrp)
{
	struct arglist al;
	size_t argc;

	template_arglist_init(&al);
	argc = template_variable_lexer(value, &al, errstrp);
	if (argc == (size_t)-1)
		return ((size_t)-1);

	if (argc == 0) {
		*errstrp = ERRSTR_EMPTY;
		return ((size_t)-1);
	}

	if (wcscmp(al.argv[0], L"path") == 0) {
		cmd_path_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"branch") == 0) {
		cmd_branch_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"color") == 0) {
		cmd_color_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"date") == 0) {
		cmd_date_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"hostname") == 0) {
		cmd_hostname_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"uid") == 0) {
		cmd_uid_exec(argc, al.argv, out, len);
	} else if (wcscmp(al.argv[0], L"sep") == 0) {
		if (!prevempty)
			cmd_sep_exec(argc, al.argv, out, len);
	} else {
		*errstrp = ERRSTR_UNKCMD;
		return ((size_t)-1);
	}

	return (wcslen(out));
}
