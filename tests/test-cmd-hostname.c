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
test_cmd_hostname_exec__short(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"hostname";
	wchar_t buf[MAX_OUTPUT_LEN];
	struct arglist al;

	strlcpy(test_hostname_value, "foobar.example.com", MAXHOSTNAMELEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	cmd_hostname_exec(al.argc, al.argv, buf, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(buf, L"foobar"));
}

static int
test_cmd_hostname_exec__long(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"hostname -l";
	wchar_t buf[MAX_OUTPUT_LEN];
	struct arglist al;

	strlcpy(test_hostname_value, "foobar.example.com", MAXHOSTNAMELEN);

	template_arglist_init(&al);
	template_variable_lexer(input, &al, &errstr);
	cmd_hostname_exec(al.argc, al.argv, buf, MAX_OUTPUT_LEN);

	return (assert_wstring_equals(buf, L"foobar.example.com"));
}
