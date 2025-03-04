/*
 * Copyright (c) 2009-2025 Bertrand Janin <b@janin.com>
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

#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <err.h>
#include <locale.h>

#include "prwd.h"
#include "config.h"
#include "alias.h"
#include "findr.h"
#include "cmd-path.h"
#include "template.h"
#include "wcslcpy.h"

extern int cfg_cleancut;
extern size_t cfg_maxpwdlen;
extern int cfg_newsgroup;
extern wchar_t cfg_template[MAX_OUTPUT_LEN];

extern int wopterr;

wchar_t	 home[MAXPATHLEN];


static void
prwd(wchar_t *t)
{
	wchar_t output[MAX_OUTPUT_LEN];
	const wchar_t *errstr;

	template_render(t, output, MAX_OUTPUT_LEN, &errstr);
	if (errstr != NULL)
		errx(1, "template error: %ls", errstr);

	wprintf(L"%ls\n", output);
}

#ifndef REGRESS
int
main(int argc, char **argv)
{
	char *t, *findr_target = NULL;
	int opt, run_dump_alias_vars = 0, run_findr = 0;

	while ((opt = getopt(argc, argv, "afF:t:Vh")) != -1) {
		switch (opt) {
		case 'a':
			run_dump_alias_vars = 1;
			break;
		case 'f':
			run_findr = 1;
			break;
		case 'F':
			run_findr = 1;
			findr_target = optarg;
			break;
		case 't':
			mbstowcs(cfg_template, optarg, MAX_OUTPUT_LEN);
			break;
		case 'V':
			puts("prwd-"VERSION);
			exit(-1);
		default:
			printf("usage: prwd [-aVh] [-t template]\n");
			exit(-1);
		}
	}

	setlocale(LC_ALL, "");

	/* Populate $HOME */
	t = getenv("HOME");
	mbstowcs(home, t, MAXPATHLEN);
	if (home == NULL || *home == L'\0')
		errx(0, "Unknown variable '$HOME'.");

	read_config();

	if (run_findr) {
		return findr(findr_target);
	}

	if (run_dump_alias_vars) {
		alias_dump_vars();
		return (0);
	}

	/* No template configured, try to get the env var. */
	if (wcslen(cfg_template) == 0 && (t = getenv("PRWD")) != NULL)
		mbstowcs(cfg_template, t, MAX_OUTPUT_LEN);

	/* Still no template, build one using legacy flags. */
	if (wcslen(cfg_template) == 0)
		template_from_config(cfg_template, MAX_OUTPUT_LEN);

	prwd(cfg_template);

	return (0);
}
#endif	// ifndef REGRESS
