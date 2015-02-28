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
test_utils__tokcpy__unchanged(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"foo") &&
	    assert_wstring_equals(output, L"foo")
	);
}

static int
test_utils__tokcpy__with_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo/bar";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"foo/bar") &&
	    assert_wstring_equals(output, L"foo")
	);
}

static int
test_utils__tokcpy__empty_string(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"") &&
	    assert_wstring_equals(output, L"")
	);
}

static int
test_utils__tokcpy__just_a_slash(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"/";
	wchar_t output[MAX_OUTPUT_LEN];
	tokcpy(input, output);
	return (
	    assert_wstring_equals(input, L"/") &&
	    assert_wstring_equals(output, L"")
	);
}
