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

#define TBUFLEN 64

static int
test_path_quickcut__empty(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 32;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L""));
}

static int
test_path_quickcut__one_to_one(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 1;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"o", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"o"));
}

static int
test_path_quickcut__one_to_two(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 2;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"o", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"o"));
}

static int
test_path_quickcut__thirty_to_ten(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"qwertyuiopasdfghjklzxcvbnmqwer", TBUFLEN,
	    maxlen, filler);

	return (assert_wstring_equals(out, L"...bnmqwer"));
}

static int
test_path_quickcut__ten_to_thirty(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 30;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"1234567890", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"1234567890"));
}

static int
test_path_quickcut__ten_to_ten(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_quickcut(out, L"1234567890", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"1234567890"));
}

static int
test_path_cleancut__empty(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L""));
}

static int
test_path_cleancut__root_to_ten(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_cleancut__root_to_one(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 1;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"/"));
}

static int
test_path_cleancut__tmp_to_one(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 1;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/tmp", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"."));
}

static int
test_path_cleancut__tmp_to_three(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 3;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/tmp", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"..."));
}

static int
test_path_cleancut__tmp_to_four(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 4;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/tmp", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"/tmp"));
}

static int
test_path_cleancut__tmp_to_ten(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/tmp", TBUFLEN, maxlen, filler);
	return (assert_wstring_equals(out, L"/tmp"));
}

static int
test_path_cleancut__uld_to_one(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 1;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/usr/local/doc", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"."));
}

static int
test_path_cleancut__uld_to_five(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 5;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/usr/local/doc", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"...oc"));
}

static int
test_path_cleancut__uld_to_ten(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 10;
	wchar_t filler[] = L"...";

	path_cleancut(out, L"/usr/local/doc", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L".../doc"));
}

static int
test_path_cleancut__uld_to_eleven(void)
{
	wchar_t out[TBUFLEN];
	size_t maxlen = 11;
	wchar_t filler[] = L"_";

	path_cleancut(out, L"/usr/local/doc", TBUFLEN, maxlen, filler);

	return (assert_wstring_equals(out, L"_/local/doc"));
}
