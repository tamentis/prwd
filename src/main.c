/*
 * Copyright (c) 2009-2015 Bertrand Janin <b@janin.com>
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
#include <wchar.h>
#include <err.h>
#include <locale.h>

#include "prwd.h"
#include "config.h"
#include "alias.h"
#include "path.h"
#include "branch.h"
#include "hostname.h"
#include "template.h"
#include "wcslcpy.h"

extern int cfg_cleancut;
extern size_t cfg_maxpwdlen;
extern int cfg_newsgroup;
extern wchar_t cfg_template[MAX_OUTPUT_LEN];

extern int wopterr;

wchar_t	 home[MAXPATHLEN];


/*
 * Main prwd functionality, prints a reduced working directory.
 */
static void
prwd(void)
{
	size_t len;
	const wchar_t *errstr;
	wchar_t wcswd[MAX_OUTPUT_LEN];

	path_wcswd(wcswd, MAX_OUTPUT_LEN, &errstr);
	if (errstr != NULL) {
		wcslcpy(wcswd, errstr, MAX_OUTPUT_LEN);
		goto done;
	}

	/* Replace the beginning with ~ for directories within $HOME. */
	alias_add(L"~", home, &errstr);
	if (errstr != NULL)
		errx(1, "failed to add default \"~\" alias: %ls", errstr);

	/* Alias handling */
	alias_replace(wcswd);

	/* If the path is still too long, crop it. */
	len = wcslen(wcswd);

done:
	wprintf(L"%ls\n", wcswd);
}

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

#ifndef REGRESS
int
main(int argc, char **argv)
{
	char *t;
	int opt, run_dump_alias_vars = 0;

	while ((opt = getopt(argc, argv, "at:Vh")) != -1) {
		switch (opt) {
		case 'a':
			run_dump_alias_vars = 1;
			break;
		case 't':
			mbstowcs(cfg_template, optarg, MAX_OUTPUT_LEN);
			break;
		case 'V':
			puts("prwd-"VERSION);
			exit(-1);
		default:
			printf("usage: prwd [-aVh]\n");
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

	if (run_dump_alias_vars) {
		alias_dump_vars();
		return (0);
	}

	if (wcslen(cfg_template) == 0) {
		if ((t = getenv("PRWD")) != NULL) {
			mbstowcs(cfg_template, t, MAX_OUTPUT_LEN);
		}
	}
	if (wcslen(cfg_template) > 0) {
		prwd_template(cfg_template);
		return (0);
	}

	prwd();

	return (0);
}
#endif	// ifndef REGRESS
