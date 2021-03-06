/*
 * Copyright (c) 2013-2015 Bertrand Janin <b@janin.com>
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

/* Maximum filler length and default filler */
#define MAX_FILLER_LEN 16
#define DEFAULT_FILLER L"..."

/* Default value for the maxpwdlen configuration setting */
#define MAXPWD_LEN 24

/* Maximum character length for branch */
#define MAX_BRANCH_LEN 32

/* Used to split various things */
#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""

/* Maximum output size */
#define MAX_OUTPUT_LEN 1024

/*
 * DEFAULT_TEMPLATE is the template used by prwd in case none was specified
 * through environment variable, configuration file or command-line.
 */
#define DEFAULT_TEMPLATE L"${hostname}:${path -l 32}${uid} "
