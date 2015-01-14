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

/*
 * This is the order of operation for template rendering:
 *
 *  1. template_tokenize() will take a full template string and turn it into a
 *     list of tokens.
 *  2. template_render() will loop over the tokens and copy or execute them
 *     depending on their type (STATIC vs DYNAMIC):
 *      2.1. template_exec() is run from render on dynamic tokens:
 *            a) template_variable_lexer() will split the dynamic token
 *               ("variable") into an arglist which is suitable for getopt().
 *            b) the commands found in the arglist are executed.
 */

#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#include <wchar.h>

/* Maximum number of tokens in a template */
#define MAX_TOKEN_COUNT 32

/* Maximum token length (wide-chars) */
#define MAX_TOKEN_LEN 64

/* Maximum Number of arguments in a single template variable */
#define MAX_ARG_COUNT 32

/* Maximum number of characters (including NUL-bytes) stored in an arglist */
#define MAX_ARGLIST_SIZE (64 * MAX_ARG_COUNT)

enum tokentype { TOKEN_STATIC, TOKEN_DYNAMIC };

struct token {
	enum tokentype type;
	wchar_t value[MAX_TOKEN_LEN];
};

/*
 * argc: count of arguments
 * len: number of significant bytes in value
 * argv: pointer to all the arguments in value
 * value: NUL-byte delimited arguments
 */
struct arglist {
	size_t argc;
	size_t len;
	wchar_t *argv[MAX_ARG_COUNT];
	wchar_t value[MAX_ARGLIST_SIZE];
};

int	 template_tokenize(wchar_t *, struct token *, size_t, const char **);
int	 template_render(wchar_t *, wchar_t *, size_t, const char **);
int	 template_exec(wchar_t *, wchar_t *, size_t, const char **);
size_t	 template_variable_lexer(wchar_t *, struct arglist *, const char **);
void	 template_arglist_init(struct arglist *);
size_t	 template_arglist_insert(struct arglist *, wchar_t *);

#endif /* ifndef _TEMPLATE_H_ */
