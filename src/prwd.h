/*
 * Copyright (c) 2013 Bertrand Janin <b@janin.com>
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

/* Maximum filler length and default filler. */
#define FILLER_LEN 16
#define FILLER_DEF L"..."

/* Default value for the maxpwdlen configuration setting. */
#define MAXPWD_LEN 24

/* Maximum number of aliases and maximum length of alias names. */
#define MAX_ALIASES 64
#define ALIAS_NAME_LEN 32

/* Maximum character length for branch and hostname. */
#define MAX_BRANCH_LEN 32
#define MAX_HOSTNAME_LEN 32

/* Used to split various things. */
#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""

/* Maximum output size. */
#define MAX_OUTPUT_LEN 1024

struct {
	wchar_t	name[ALIAS_NAME_LEN];
	wchar_t	path[MAXPATHLEN];
} aliases[MAX_ALIASES];

/* Types of source control mechanisms */
enum version_control_system {
	VCS_NONE,
	VCS_MERCURIAL,
	VCS_GIT
};
