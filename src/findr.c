/*
 * Copyright (c) 2025 Bertrand Janin <bertrand@janin.com>
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
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "findr.h"
#include "prwd.h"
#include "utils.h"
#include "wcslcpy.h"


static int
findr_target(char *target_filename)
{
	char *c, pwd[MAXPATHLEN], path[MAXPATHLEN];

	if (target_filename == NULL) {
		return 0;
	}

	if (getcwd(pwd, MAXPATHLEN) == NULL) {
		errx(1, "unable to get current path");
	}

	for (;;) {
		snprintf(path, MAXPATHLEN, "%s/%s", pwd, target_filename);
		if (path_is_valid(path)) {
			printf("%s\n", pwd);
			return 1;
		}
		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	}

	return 0;
}


static int
findr_repository(void)
{
	char *c, pwd[MAXPATHLEN];

	if (getcwd(pwd, MAXPATHLEN) == NULL) {
		errx(1, "unable to get current path");
	}

	for (;;) {
		const char *suffixes[] = {".hg", ".git"};
		char path[MAXPATHLEN];

		for (size_t i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++) {
			snprintf(path, MAXPATHLEN, "%s/%s", pwd, suffixes[i]);
			if (path_is_valid(path)) {
				printf("%s\n", pwd);
				return 1;
			}
		}

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	}

	return 0;
}


static int
findr_readme(void)
{
	char *c, pwd[MAXPATHLEN];

	if (getcwd(pwd, MAXPATHLEN) == NULL) {
		errx(1, "unable to get current path");
	}

	for (;;) {
		const char *suffixes[] = {"README", "README.md", "README.txt"};
		char path[MAXPATHLEN];

		for (size_t i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++) {
			snprintf(path, MAXPATHLEN, "%s/%s", pwd, suffixes[i]);
			if (path_is_valid(path)) {
				printf("%s\n", pwd);
				return 1;
			}
		}

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	}

	return 0;
}


/*
 * Find the nearest parent considered a project root and print that path
 *
 * This function implements the `prwd -f [target]` functionality.
 *
 * The only parameter may be NULL if no parameter was provided.
 */
int
findr(char *target_filename)
{
	if (findr_target(target_filename) || findr_repository() || findr_readme()) {
		return 0;
	}

	return 1;
}
