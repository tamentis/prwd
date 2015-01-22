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

static int
test_path_newsgroupize__null(void)
{
	wchar_t out[64];
	path_newsgroupize(out, NULL, 64);
	return (1);
}

static int
test_path_newsgroupize__empty(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"", 64);
	return (assert_wstring_equals(out, L""));
}

static int
test_path_newsgroupize__one(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"a", 64);
	return (assert_wstring_equals(out, L"a"));
}

static int
test_path_newsgroupize__root(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/", 64);
	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_newsgroupize__slash_one(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/a", 64);
	return (assert_wstring_equals(out, L"/a"));
}

static int
test_path_newsgroupize__tmp(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo", 64);
	return (assert_wstring_equals(out, L"/foo"));
}

static int
test_path_newsgroupize__home(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo/bar", 64);
	return (assert_wstring_equals(out, L"/f/bar"));
}

static int
test_path_newsgroupize__shortpath(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"~/foo/bar", 64);
	return (assert_wstring_equals(out, L"~/f/bar"));
}

static int
test_path_newsgroupize__shortpath_one_level(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"~/foo", 64);
	return (assert_wstring_equals(out, L"~/foo"));
}

static int
test_path_newsgroupize__alreadyshort(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/a/b/c/d/e/f/g/h/i/j", 64);
	return (assert_wstring_equals(out, L"/a/b/c/d/e/f/g/h/i/j"));
}

static int
test_path_newsgroupize__trailingslash(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"/foo/bar/", 64);
	return (assert_wstring_equals(out, L"/f/bar/"));
}

static int
test_path_newsgroupize__alias(void)
{
	wchar_t out[64];
	path_newsgroupize(out, L"$alias/foo/bar/baz", 64);
	return (assert_wstring_equals(out, L"$alias/f/b/baz"));
}
