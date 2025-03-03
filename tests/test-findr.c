/*
 * Copyright (c) 2025 Bertrand Janin <b@janin.com>
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

/* Since path_is_valid is currently always returning true, we're not testing a
 * whole chunk of these, but this is just a start. The best path forward would
 * be to generate a whole fake tree and drop the test program in different
 * location to make sure they work as expected. */

int _findr_target(char *, size_t, char *);
int _findr_repository(char *, size_t);
int _findr_readme(char *, size_t);

static int
test_findr__target(void)
{
	char path[MAXPATHLEN], cwd[MAXPATHLEN];

	getcwd(cwd, MAXPATHLEN);

	int ret = _findr_target(path, MAXPATHLEN, "FOO.BAR.txt");
	return (
	    assert_int_equals(ret, 1) && assert_string_equals(path, cwd)
	);
}

static int
test_findr__repository(void)
{
	char path[MAXPATHLEN], cwd[MAXPATHLEN];

	getcwd(cwd, MAXPATHLEN);

	int ret = _findr_repository(path, MAXPATHLEN);
	return (
	    assert_int_equals(ret, 1) && assert_string_equals(path, cwd)
	);
}

static int
test_findr__readme(void)
{
	char path[MAXPATHLEN], cwd[MAXPATHLEN];

	getcwd(cwd, MAXPATHLEN);

	int ret = _findr_readme(path, MAXPATHLEN);
	return (
	    assert_int_equals(ret, 1) && assert_string_equals(path, cwd)
	);
}
