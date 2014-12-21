/*
 * Copyright (c) 2013-2014 Bertrand Janin <b@janin.com>
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

#include "alias.h"
#include "prwd.h"
#include "utils.h"
#include "wcslcpy.h"


struct alias aliases[MAX_ALIASES];
int alias_count = 0;


/*
 * Add a new alias to the stack.  If errstrp is not NULL after returning, an
 * error occured and the alias was not added.
 */
void
alias_add(wchar_t *name, wchar_t *path, const char **errstrp)
{
	*errstrp = NULL;

	if (alias_count >= MAX_ALIASES - 1) {
		*errstrp = "too many aliases";
		return;
	}

	if (wcslen(path) > (MAXPATHLEN - 1)) {
		*errstrp = "alias path is too long";
		return;
	}

	if (wcslen(path) < wcslen(name)) {
		*errstrp = "alias name longer than its path";
		return;
	}

	if (wcschr(name, '/') != NULL) {
		*errstrp = "alias name contains '/'";
		return;
	}

	wcslcpy(aliases[alias_count].name, name, ALIAS_NAME_LEN);
	wcslcpy(aliases[alias_count].path, path, MAXPATHLEN);
	alias_count++;
}

/*
 * Remove all the aliases in the list.  This is used by the test suite.  The
 * list on the stack, no need to free() anything.
 */
void
alias_purge_all()
{
	alias_count = 0;
}

/*
 * Return an alias given its name or NULL if not found.
 */
struct alias *
alias_get(wchar_t *name)
{
	int i;
	struct alias *alias = NULL;

	for (i = 0; i < alias_count; i++) {
		if (wcscmp(aliases[i].name, name) == 0) {
			alias = &aliases[i];
			break;
		}
	}

	return (alias);
}

/*
 * Expands aliases at the beginning of the string.  For example "$foo/fruits"
 * could become "/var/lib/foo/fruits".
 */
void
alias_expand_prefix(wchar_t *input, wchar_t *output)
{
	struct alias *alias;
	wchar_t name[MAX_OUTPUT_LEN];
	size_t i = 0;

	tokcpy(input, name);

	alias = alias_get(name);
	if (alias == NULL)
		goto finish;

	i = wcslcpy(output, alias->path, MAX_OUTPUT_LEN);
	if (i >= MAX_OUTPUT_LEN)
		return;

	input += wcslen(alias->name);

finish:
	wcslcpy(output + i, input, MAX_OUTPUT_LEN - i);
}

/*
 * Returns the best fitted alias to use as prefix for the provided path. For
 * example, given a path such as "/var/lib/foo" and the following aliases:
 *      lib        /var/lib
 *      foo        /var/lib/foo
 * This function would return the "foo" alias since its path replaces a larger
 * amount of characters, reducing the on-screen path the most.  This function
 * returns NULL if no alias was found.
 */
struct alias *
alias_get_by_path(wchar_t *path)
{
	struct alias *alias = NULL;
	size_t len, max;
	int i;

	max = 0;
	for (i = 0; i < alias_count; i++) {
		len = wcslen(aliases[i].path);
		if (wcsncmp(aliases[i].path, path, len) == 0) {
			if (len > max) {
				alias = &aliases[i];
				max = len;
			}
		}
	}

	return (alias);
}

/*
 * Dump all the aliases starting with $ as shell variable.  This output is
 * meant to be used with eval in your profile file.
 *
 * This function does not attempt to shell escape directories, if you have
 * paths on your computer containing double-quotes, dollar signs, then I feel
 * sorry for you.
 */
void
alias_dump_vars(void)
{
	int i;
	wchar_t path[MAX_OUTPUT_LEN], output[MAX_OUTPUT_LEN];

	for (i = 0; i < alias_count; i++) {
		if (aliases[i].name[0] == '$') {
			wcslcpy(path, aliases[i].path, MAX_OUTPUT_LEN);
			alias_expand_prefix(path, output);
			if (!wc_file_exists(output))
				continue;
			/* Skip the '$' */
			wprintf(L"export %ls=\"%ls\"\n", aliases[i].name + 1,
					output);
		}
	}
}

/*
 * Find the best match to get the shortest path as possible.
 */
void
alias_replace(wchar_t *path)
{
	size_t nlen, plen;
	wchar_t buf[MAX_OUTPUT_LEN];
	struct alias *alias;

	alias = alias_get_by_path(path);
	if (alias == NULL)
		return;

	plen = wcslen(alias->path);
	nlen = wcslcpy(buf, alias->name, MAX_OUTPUT_LEN);
	wcslcpy(buf + nlen, path + plen, MAX_OUTPUT_LEN - nlen);
	wcslcpy(path, buf, MAX_OUTPUT_LEN);
}
