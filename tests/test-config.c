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
test_config__process_config_line__set_no_var(void)
{
	wchar_t line[] = L"set";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr, L"set without variable name"));
}

static int
test_config__process_config_line__alias_no_name(void)
{
	wchar_t line[] = L"alias";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr, L"alias without name"));
}

static int
test_config__process_config_line__just_spaces(void)
{
	wchar_t line[] = L"        ";
	process_config_line(line, &errstr);
	return (assert_null(errstr));
}

static int
test_config__process_config_line__comments(void)
{
	wchar_t line[] = L"# comments";
	process_config_line(line, &errstr);
	return (assert_null(errstr));
}

static int
test_config__process_config_line__set_maxlength_250(void)
{
	wchar_t line[] = L"set maxlength 250";
	process_config_line(line, &errstr);
	return (assert_null(errstr) &&
	    assert_int_equals(cfg_maxpwdlen, 250));
}

static int
test_config__process_config_line__set_maxlength_bad(void)
{
	wchar_t line[] = L"set maxlength $F@#$";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr,
	    L"invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_overflow(void)
{
	wchar_t line[] = L"set maxlength 5000";
	process_config_line(line, &errstr);
	return (assert_wstring_equals(errstr,
	    L"invalid number for set maxlength"));
}

static int
test_config__process_config_line__set_maxlength_quoted(void)
{
	wchar_t line[] = L"set maxlength \"50\"";
	process_config_line(line, &errstr);
	return (
	    assert_null(errstr) &&
	    assert_int_equals(cfg_maxpwdlen, 50)
	);
}
