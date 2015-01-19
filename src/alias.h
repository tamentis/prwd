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

#ifndef _ALIAS_H_
#define _ALIAS_H_

#include <sys/param.h>

#include <wchar.h>

#define MAX_ALIASES 64
#define ALIAS_NAME_LEN 32

struct alias {
	wchar_t	name[ALIAS_NAME_LEN];
	wchar_t	path[MAXPATHLEN];
};

void		 alias_add(wchar_t *, wchar_t *, const wchar_t **);
void		 alias_purge_all(void);
void		 alias_expand_prefix(wchar_t *, wchar_t *);
void		 alias_dump_vars(void);
struct alias 	*alias_get(wchar_t *);
struct alias	*alias_get_by_path(wchar_t *);
void		 alias_replace(wchar_t *);

#endif /* ifndef _ALIAS_H_ */
