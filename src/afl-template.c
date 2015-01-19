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

#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <err.h>
#include <locale.h>

#include "prwd.h"
#include "template.h"

extern wchar_t cfg_template[MAX_OUTPUT_LEN];
wchar_t	 home[MAXPATHLEN];


static void
prwd_template(wchar_t *t)
{
	wchar_t output[MAX_OUTPUT_LEN];
	int i;
	const wchar_t *errstr;

	i = template_render(t, output, MAX_OUTPUT_LEN, &errstr);
	if (errstr != NULL)
		errx(1, "template error: %ls", errstr);

	wprintf(L"%ls\n", output);
}

int
main(void)
{
	char buf[4096];

	setlocale(LC_ALL, "");

	fread(buf, sizeof(char), 4096, stdin);
	mbstowcs(cfg_template, buf, MAX_OUTPUT_LEN);

	prwd_template(cfg_template);

	return (0);
}
