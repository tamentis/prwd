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

#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

#include "branch.h"
#include "utils.h"

/* How much to read of the branch file (e.g. HEAD, .hg/branch, etc.) */
#define BRANCH_FILE_BUFSIZE 1024

/*
 * Extract a branch name from *data and save it to *out.  Since the .hg/branch
 * file is a simple branch name, we only need to remove a potential new-line
 * character.
 */
void
parse_hg_branch(wchar_t *out, char *data, size_t len)
{
	char *c;

	if ((c = strchr(data, '\n')) != NULL)
		*c = '\0';

	mbstowcs(out, data, len);
}

/*
 * Extract a git branch name from *data and save it to *out.
 */
void
parse_git_head(wchar_t *out, char *data, size_t len)
{
	char *c;

	/* This is a branch head, just print the branch. */
	if (strncmp(data, "ref: refs/heads/", 16) == 0) {
		data += 16;
		goto done;
	}

	/* Show all other kinds of ref as-is. */
	if (strncmp(data, "ref:", 4) == 0) {
		data += 5;
		goto done;
	}

	/*
	 * Anything that is not a direct ref should be a plain changeset id,
	 * trim the id to 6 characters.  It's not very useful but at least
	 * gives you a clue that you're not on a branch and you can see when
	 * the changeset changes.
	 */
	strlcpy(data + 6, "...", len - 6);

done:
	if ((c = strchr(data, '\n')) != NULL)
		*c = '\0';

	mbstowcs(out, data, len);
}

/*
 * This module should never crash and will always return a value on *out.
 * Errors are communicated back to the end-user via *out in a maner that should
 * help them understand or correct the issue.
 */
void
branch_exec(int argc, wchar_t **argv, wchar_t *out, size_t len)
{
	FILE *fp;
	char *c, pwd[MAXPATHLEN], path[MAXPATHLEN], buf[BRANCH_FILE_BUFSIZE];
	size_t s;
	enum vcs_types type = VCS_NONE;
	(void)argc;
	(void)argv;

	/*
	 * Recurse our way up from the current dir and find clues that we are
	 * within a source control repository.
	 */
	if (getcwd(pwd, MAXPATHLEN) == NULL) {
		wcslcpy(out, L"<branch-cwd-error>", len);
		return;
	}

	for (;;) {
		snprintf(path, MAXPATHLEN, "%s/.hg/branch", pwd);
		if (path_is_valid(path)) {
			type = VCS_MERCURIAL;
			break;
		}

		snprintf(path, MAXPATHLEN, "%s/.git/HEAD", pwd);
		if (path_is_valid(path)) {
			type = VCS_GIT;
			break;
		}

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	}

	if (type == VCS_NONE) {
		out[0] = L'\0';
		return;
	}

	fp = fopen(path, "r");
	if (fp == NULL) {
		wcslcpy(out, L"<branch-io-error>", len);
		return;
	}

	s = fread(buf, 1, BRANCH_FILE_BUFSIZE, fp);
	fclose(fp);
	buf[s] = '\0';

	switch (type) {
	case VCS_MERCURIAL:
		parse_hg_branch(out, buf, len);
		break;
	case VCS_GIT:
		parse_git_head(out, buf, len);
		break;
	default:
		wcslcpy(out, L"<branch-bad-vcs>", len);
		break;
	}
}
