/*
 * Copyright (c) 2009-2014 Bertrand Janin <b@janin.com>
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
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <err.h>

#include "prwd.h"
#include "vcs.h"
#include "strlcpy.h"
#include "wcslcpy.h"

/*
 * Recurse up from $PWD to find a .hg/ directory with a valid branch file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
static size_t
get_mercurial_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c, pwd[MAX_OUTPUT_LEN], path[MAXPATHLEN], buf[MAX_BRANCH_LEN];
	size_t branch_size;
	struct stat bufstat;
	int found = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

	do {
		snprintf(path, MAXPATHLEN, "%s/.hg/branch", pwd);

		found = stat(path, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found != 0 && path[1] != '\0');

	if (found == -1)
		return (0);

	fp = fopen(path, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		return (mbstowcs(dst, buf, MAX_BRANCH_LEN));
	}

	if (fread(buf, 1, size, fp) == 0)
		err(1, "failed to read the .hg/branch file");
	fclose(fp);

	/* remove the trailing new line if any */
	if ((c = strchr(buf, '\n')) != NULL)
		*c = '\0';

	branch_size = mbstowcs(dst, buf, MAX_BRANCH_LEN);

	return (branch_size);
}

/*
 * Recurse up from $PWD to find a .git/ directory with a valid HEAD file,
 * read this file, copy the branch name in dst, up to a maximum of 'size'
 * and return the amount of bytes copied.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
static size_t
get_git_branch(wchar_t *dst, size_t size)
{
	FILE *fp;
	char *c, path[MAXPATHLEN], buf[MAX_BRANCH_LEN], pwd[MAX_OUTPUT_LEN];
	size_t s;
	struct stat bufstat;
	int found = -1;

	/* start from the working dir */
	strlcpy(pwd, getcwd(NULL, MAX_OUTPUT_LEN), MAX_OUTPUT_LEN);

	do {
		snprintf(path, MAXPATHLEN, "%s/.git/HEAD", pwd);

		found = stat(path, &bufstat);

		if ((c = strrchr(pwd, '/')) == NULL)
			break;

		*c = '\0';
	} while (found != 0 && path[1] != '\0');

	if (found == -1)
		return (0);

	fp = fopen(path, "r");
	if (fp == NULL) {
		strlcpy(buf, "###", 4);
		c = buf;
		goto finish;
	}

	s = fread(buf, 1, size, fp);
	fclose(fp);

	buf[MAX_BRANCH_LEN - 1] = '\0';

	/* This is a branch head, just print the branch. */
	if (strncmp(buf, "ref: refs/heads/", 16) == 0) {
		c = strchr(buf, '\n');
		if (c)
			*(c) = '\0';
		c = buf + 16;
		goto finish;
	}

	/* Show all other kinds of ref as-is (does it even exist?) */
	if (strncmp(buf, "ref:", 4) == 0) {
		c = strchr(buf, '\n');
		if (c)
			*(c) = '\0';
		c = buf + 5;
		goto finish;
	}

	/* That's probably just a changeset, just show the first 6 chars. */
	if (s > 6) {
		strlcpy(buf + 6, "...", 4);
		c = buf;
		goto finish;
	}

	/* We shouldn't get there, but we mind as well no crash. */
	strlcpy(buf, "???", 4);
	c = buf;

finish:
	return (mbstowcs(dst, c, MAX_BRANCH_LEN));
}

/*
 * Add the mercurial branch at the beginning of the path. If a branch was
 * found, 1 is returned else 0.
 *
 * Input:  /home/bjanin/prwd
 * Output: prwd-1.3:/home/bjanin/prwd
 */
int
add_branch(wchar_t *s, enum vcs_types vcs)
{
	wchar_t org[MAX_OUTPUT_LEN];
	wchar_t branch[MAX_BRANCH_LEN];
	size_t len;

	wcslcpy(org, s, MAX_OUTPUT_LEN);

	switch (vcs) {
		case VCS_MERCURIAL:
			if (get_mercurial_branch(branch, MAX_BRANCH_LEN) == 0)
				return (0);
			break;
		case VCS_GIT:
			if (get_git_branch(branch, MAX_BRANCH_LEN) == 0)
				return (0);
			break;
		default:
			return (0);
	}

	len = wcslcpy(s, branch, MAX_BRANCH_LEN);

	s += len;
	*(s++) = ':';
	wcslcpy(s, org, MAX_OUTPUT_LEN - len - 1);

	return (1);
}
