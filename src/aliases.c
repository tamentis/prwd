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

#include <sys/param.h>

#include <wchar.h>

#include "prwd.h"
#include "wcslcpy.h"
#include "utils.h"


int alias_count = 0;
struct alias_t aliases[MAX_ALIASES];

extern wchar_t home[MAXPATHLEN];


/*
 * Add a new alias to the list.
 */
void
add_alias(wchar_t *name, wchar_t *value, int linenum)
{
	if (wcslen(value) > (MAXPATHLEN - 1)) {
		fatal("prwdrc:%d: alias path is too long (MAXPATHLEN=%d).\n",
				linenum, MAXPATHLEN);
		return;
	}

	if (wcslen(value) < wcslen(name)) {
		fatal("prwdrc:%d: alias name should not be longer than the "
				"value.\n", linenum);
		return;
	}

	if (wcschr(name, '/') != NULL) {
		fatal("prwdrc:%d: alias name should not contain any '/' "
				"(slash).\n", linenum);
		return;
	}

	if (alias_count >= MAX_ALIASES - 1) {
		fatal("prwdrc:%d: you have reached the %d aliases limit.\n",
				linenum, MAX_ALIASES);
		return;
	}

	wcslcpy(aliases[alias_count].name, name, ALIAS_NAME_LEN);
	wcslcpy(aliases[alias_count].path, value, MAXPATHLEN);
	alias_count++;
}


/*
 * Remove all the aliases in the list.
 *
 * This is mostly used by the test suite at this point. Since the list is all
 * pre-allocated there is no need to free() anything.
 */
void
purge_aliases()
{
	alias_count = 0;
}


/*
 * At this point, we only return aliases resolving to real paths.
 */
wchar_t *
get_path_for_alias(wchar_t *alias, int len)
{
	int i;

	for (i = 0; i < alias_count; i++) {
		if (!wc_file_exists(aliases[i].path))
			continue;
		if (wcsncmp(aliases[i].name, alias, len) == 0) {
			return aliases[i].path;
		}
	}

	return NULL;
}


/*
 * Expands the aliases within the given string.
 */
void
expand_aliases(wchar_t *input, int size)
{
	wchar_t buffer[MAX_OUTPUT_LEN] = L"";
	wchar_t *c, *end_of_token, *path;
	int i, len, available;

	/* 'c' follows the input */
	c = input;

	for (i = 0; i < MAX_OUTPUT_LEN && *c != L'\0'; i++) {
		/* Standard copy until we find a '$'. */
		if (*c != L'$')
			goto standard_copy;

		/* Find the end of the possible token. */
		end_of_token = wcschr(c, L'/');
		if (end_of_token == NULL)
			end_of_token = wcschr(c, L'\0');

		/* Is this a valid alias? */
		len = end_of_token - c;
		path = get_path_for_alias(c, len);
		if (path == NULL)
			goto standard_copy;

		c += len;
		available = MAX_OUTPUT_LEN - i;
		len = wcslcpy(buffer + i, path, available);
		if (available < len)
			len = available;

		/* Shift one char less since we don't want to keep the NUL. */
		i += len - 1;
		continue;

standard_copy:
		buffer[i] = *c;
		c++;
	}

	buffer[i] = L'\0';

	wcslcpy(input, buffer, MAX_OUTPUT_LEN);
}
