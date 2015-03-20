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

#ifndef _PATH_H_
#define _PATH_H_

#include <wchar.h>

enum {
	ERR_NO_ACCESS = 1,
	ERR_NOT_FOUND,
	ERR_BAD_CHARSET,
	ERR_BAD_ARG,
	ERR_GENERIC
};

void	 path_wcswd(wchar_t *, size_t, const wchar_t **);
void	 cmd_path_exec(int, wchar_t **, wchar_t *, size_t);
void	 path_newsgroupize(wchar_t *, const wchar_t *, size_t);
void	 path_cleancut(wchar_t *, wchar_t *, size_t, size_t, wchar_t *);
void	 path_quickcut(wchar_t *, wchar_t *, size_t, size_t, wchar_t *);
#endif /* #ifndef _PATH_H_ */
