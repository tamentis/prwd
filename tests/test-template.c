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
test_template_tokenize__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__one_static(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_wstring_equals(tokens[0].value, L"foo") &&
	    assert_int_equals(tokens[0].type, TOKEN_STATIC) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__one_command(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"${bar}";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_wstring_equals(tokens[0].value, L"bar") &&
	    assert_int_equals(tokens[0].type, TOKEN_COMMAND) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__complex(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo ${bar} and fooba${r}";
	struct token tokens[10];
	int i;

	i = template_tokenize(input, tokens, 10, &errstr);

	return (
	    assert_int_equals(i, 4) &&
	    assert_wstring_equals(tokens[0].value, L"foo ") &&
	    assert_wstring_equals(tokens[1].value, L"bar") &&
	    assert_wstring_equals(tokens[2].value, L" and fooba") &&
	    assert_wstring_equals(tokens[3].value, L"r") &&
	    assert_int_equals(tokens[0].type, TOKEN_STATIC) &&
	    assert_int_equals(tokens[1].type, TOKEN_COMMAND) &&
	    assert_int_equals(tokens[2].type, TOKEN_STATIC) &&
	    assert_int_equals(tokens[3].type, TOKEN_COMMAND) &&
	    assert_null(errstr)
	);
}

static int
test_template_tokenize__too_many_tokens(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo ${bar} and fooba${r}";
	struct token tokens[2];
	int i;

	i = template_tokenize(input, tokens, 2, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr, L"too many tokens")
	);
}

static int
test_template_tokenize__token_too_long(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foobarfoobarfoobarfoobarfoobar"\
					L"foobarfoobarfoobarfoobarfoobar"\
					L"foobarfoobarfoobarfoobarfoobar"\
					L"foobarfoobarfoobarfoobarfoobar"\
					L"foobarfoobarfoobarfoobarfoobar";
	struct token tokens[2];
	int i;

	i = template_tokenize(input, tokens, 2, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr, L"invalid token size")
	);
}

static int
test_template_render__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	wchar_t output[MAX_OUTPUT_LEN];
	int i;

	i = template_render(input, output, MAX_OUTPUT_LEN, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(output, L"")
	);
}

static int
test_template_render__just_static(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo bar and foobaz";
	wchar_t output[MAX_OUTPUT_LEN];
	int i;

	i = template_render(input, output, MAX_OUTPUT_LEN, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(output, L"foo bar and foobaz")
	);
}

static int
test_template_arglist__init(void)
{
	struct arglist al;

	template_arglist_init(&al);

	return (assert_int_equals(al.argc, 0));
}

static int
test_template_arglist__insert_one(void)
{
	struct arglist al;
	size_t argc;

	template_arglist_init(&al);
	argc = template_arglist_insert(&al, L"foo");

	return (
	    assert_int_equals(argc, 1) &&
	    assert_int_equals(al.argc, 1) &&
	    assert_wstring_equals(al.argv[0], L"foo")
	);
}

static int
test_template_arglist__insert_many(void)
{
	struct arglist al;
	size_t argc;

	template_arglist_init(&al);
	argc = template_arglist_insert(&al, L"foo");
	argc = template_arglist_insert(&al, L"bar");
	argc = template_arglist_insert(&al, L"baz");

	return (
	    assert_int_equals(argc, 3) &&
	    assert_int_equals(al.argc, 3) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar") &&
	    assert_wstring_equals(al.argv[2], L"baz")
	);
}

static int
test_template_arglist__insert_err_too_many_args(void)
{
	struct arglist al;
	size_t argc;
	wchar_t s[] = L"foobar0";
	int i, a;

	template_arglist_init(&al);

	/* Add just enough arguments to border the limit */
	for (i = 0; i < MAX_ARG_COUNT; i++) {
		argc = template_arglist_insert(&al, s);
		if (argc == (size_t)-1) {
			return (0);
		}
	}

	a = (
	    assert_int_equals(argc, MAX_ARG_COUNT) &&
	    assert_int_equals(al.argc, MAX_ARG_COUNT) &&
	    assert_wstring_equals(al.argv[0], s) &&
	    assert_wstring_equals(al.argv[MAX_ARG_COUNT - 1], s)
	);

	if (!a)
		return (a);

	/* This final insert should cause an error. */
	argc = template_arglist_insert(&al, s);
	return (assert_size_t_equals(argc, (size_t)-1));
}

static int
test_template_arglist__insert_err_too_many_chars(void)
{
	struct arglist al;
	size_t i, argc = 0, max;
	wchar_t s[] = L"foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" \
		      L"foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar";
	int a;

	template_arglist_init(&al);

	max = MAX_ARGLIST_SIZE / (wcslen(s) + 1);

	for (i = 0; i < max; i++) {
		argc = template_arglist_insert(&al, s);
		if (argc == (size_t)-1) {
			break;
		}
	}

	a = (
	    assert_size_t_equals(argc, max) &&
	    assert_size_t_equals(al.argc, max) &&
	    assert_wstring_equals(al.argv[0], s) &&
	    assert_wstring_equals(al.argv[max - 1], s)
	);

	if (!a)
		return (a);

	/* This final insert should cause an error. */
	argc = template_arglist_insert(&al, s);
	return (assert_size_t_equals(argc, (size_t)-1));
}

static int
test_template_variable_lexer__empty(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 0) &&
	    assert_null(errstr)
	);
}

static int
test_template_variable_lexer__one(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foobar";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foobar")
	);
}

static int
test_template_variable_lexer__two(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo bar";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar")
	);
}

static int
test_template_variable_lexer__trailing_spaces(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo ";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 1) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo")
	);
}

static int
test_template_variable_lexer__quoted_space(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar baz")
	);
}

static int
test_template_variable_lexer__quoted_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar\\\"baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"bar\"baz")
	);
}

static int
test_template_variable_lexer__quoted_double_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar\"\"baz\"";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, 2) &&
	    assert_null(errstr) &&
	    assert_wstring_equals(al.argv[0], L"foo") &&
	    assert_wstring_equals(al.argv[1], L"barbaz")
	);
}

static int
test_template_variable_lexer__unmatched_quote(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"foo \"bar";
	size_t i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_size_t_equals(i, (size_t)-1) &&
	    assert_wstring_equals(errstr, L"unmatched quote")
	);
}

static int
test_template_variable_lexer__large_arg(void)
{
	wchar_t input[MAX_OUTPUT_LEN + 1];
	size_t i;
	struct arglist al;

	wmemset(input, L'?', MAX_OUTPUT_LEN + 1);
	input[MAX_OUTPUT_LEN] = L'\0';

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (assert_size_t_equals(i, 1));
}

static int
test_template_variable_lexer__err_arg_size(void)
{
	wchar_t input[MAX_ARGLIST_SIZE * 2 + 1];
	size_t i;
	struct arglist al;

	wmemset(input, L'?', MAX_ARGLIST_SIZE * 2 + 1);
	input[MAX_ARGLIST_SIZE * 2] = L'\0';

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_size_t_equals(i, (size_t)-1) &&
	    assert_wstring_equals(errstr, L"argument list too large")
	);
}

static int
test_template_variable_lexer__err_too_many(void)
{
	wchar_t input[MAX_OUTPUT_LEN] = L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a " \
					L"f o o b a r f o o b a r f o o b a ";
	int i;
	struct arglist al;

	template_arglist_init(&al);
	i = template_variable_lexer(input, &al, &errstr);

	return (
	    assert_int_equals(i, -1) &&
	    assert_wstring_equals(errstr, L"argument list too large")
	);
}
