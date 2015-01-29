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
test_alias__replace__none(void)
{
	wchar_t pwd[] = L"/usr/src";
	alias_purge_all();
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"/usr/src"));
}

static int
test_alias__replace__home_alone(void)
{
	wchar_t pwd[] = L"/home/foo";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/foo");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~"));
}

static int
test_alias__replace__home_and_one(void)
{
	wchar_t pwd[] = L"/home/foo/x";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/foo");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~/x"));
}

static int
test_alias__replace__home_and_tree(void)
{
	wchar_t pwd[] = L"/home/foo/x/projects/prwd";
	alias_purge_all();
	ALIAS_ADD(L"~", L"/home/foo");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"~/x/projects/prwd"));
}

static int
test_alias__replace__five_unmatching_aliases(void)
{
	wchar_t pwd[] = L"/home/foo/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"a1", L"/the/first/path");
	ALIAS_ADD(L"b2", L"/path/second");
	ALIAS_ADD(L"c3", L"/third/path");
	ALIAS_ADD(L"d4", L"foo/bar/fourth/path");
	ALIAS_ADD(L"e5", L"/home/föö");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"/home/foo/x/projects"));
}

static int
test_alias__replace__duplicate_aliases(void)
{
	wchar_t pwd[] = L"/home/foo/x/projects";
	alias_purge_all();
	ALIAS_ADD(L"aa", L"/home/foo");
	ALIAS_ADD(L"aa", L"/home/foo");
	ALIAS_ADD(L"aa", L"/home/foo");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"aa/x/projects"));
}

static int
test_alias__replace__find_smallest(void)
{
	wchar_t pwd[] = L"/home/foo/x/y/z/projects/prwd";
	alias_count = 0;
	ALIAS_ADD(L"bad1", L"/home/foo");
	ALIAS_ADD(L"bad2", L"/home");
	ALIAS_ADD(L"bad3", L"/home/foo/x");
	ALIAS_ADD(L"good", L"/home/foo/x/y/z");
	alias_replace(pwd);
	return (assert_wstring_equals(pwd, L"good/projects/prwd"));
}

static int
test_alias__add__too_many(void)
{
	int i;
	alias_purge_all();
	for (i = 0; i < MAX_ALIASES * 2; i++) {
		ALIAS_ADD(L"aa", L"/home/foo");
	}

	alias_add(L"aa", L"/home/foo", &errstr);
	if (errstr == NULL) {
		snprintf(details, sizeof(details),
		    "alias_add should have returned an error");
		return (0);
	}

	return (assert_wstring_equals(errstr, L"too many aliases"));
}

static int
test_alias__expand_prefix__normal(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$local/man/cat1";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$foo", L"/usr/local");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/usr/local/man/cat1"));
}

static int
test_alias__expand_prefix__with_slash(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$local/man/cat1/";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$local", L"/usr/local/");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/usr/local//man/cat1/"));
}

static int
test_alias__expand_prefix__single(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"$local";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$local", L"/usr/local");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"/usr/local"));
}

/*
 * The configured alias does not match anything in the path and the output
 * should be identical to the input.
 */
static int
test_alias__expand_prefix__no_alias(void)
{
	wchar_t s[MAX_OUTPUT_LEN] = L"local";
	wchar_t output[MAX_OUTPUT_LEN];

	alias_purge_all();
	ALIAS_ADD(L"$local", L"/usr/local");
	alias_expand_prefix(s, output);
	return (assert_wstring_equals(output, L"local"));
}
