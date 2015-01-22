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

#include <string.h>
#include <wchar.h>
#include <locale.h>

#include "alias.h"
#include "config.h"
#include "utils.h"
#include "prwd.h"
#include "template.h"
#include "strlcpy.h"
#include "wcslcpy.h"
#include "cmd-path.h"
#include "cmd-hostname.h"

#define RUN_TEST(f)						\
	printf("%-60s", #f);					\
	fflush(stdout);						\
	errstr = NULL;						\
	if (f()) {						\
		printf("PASS\n");				\
		passed++;					\
	} else {						\
		printf("FAIL\n");				\
		if (strlen(details)) {				\
			puts(details);				\
			details[0] = '\0';			\
		}						\
		if (errstr != NULL) {				\
			printf("%17s%ls\n", "errstr=", errstr);	\
		}						\
		failed++;					\
	};							\
	tested++;						\

#define ALIAS_ADD(a, b)						\
	alias_add(a, b, &errstr);				\
	if (errstr != NULL) {					\
		return (1);					\
	}							\

extern wchar_t cfg_filler[MAX_FILLER_LEN];
extern size_t cfg_maxpwdlen;
extern int alias_count;
const wchar_t *errstr;
char details[256] = "";
char test_hostname_value[MAXHOSTNAMELEN];
int tested = 0;
int passed = 0;
int failed = 0;
int test_file_exists = 1;

/* Used in the below path_wcswd() override. */
wchar_t path_wcswd_fakepwd[MAXPATHLEN] = L"/tmp";

/*
 * Override with predictable path.
 */
void
path_wcswd(wchar_t *wcswd, size_t len, const wchar_t **errstr)
{
	(void)errstr;
	wcslcpy(wcswd, path_wcswd_fakepwd, len);
}

/*
 * Override gethostname(3) to make the hostname predictable.
 */
int
lgethostname(char *name, size_t namelen)
{
	strlcpy(name, test_hostname_value, namelen);
	return (0);
}

/*
 * Override file_exists to ensure predictable returns.
 */
int
path_is_valid(char *filepath)
{
	(void)filepath;
	return (test_file_exists);
}

/*
 * Testing assertions with detailed output.
 */
static int
assert_string_equals(const char *value, const char *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (strcmp(value, expected) == 0)
		return (2);

bad:
	snprintf(details, sizeof(details),
	    "    strings do not match:\n"
	    "           value=\"%s\"\n"
	    "        expected=\"%s\"", value, expected);
	return (0);
}

static int
assert_wstring_equals(const wchar_t *value, const wchar_t *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (wcscmp(value, expected) == 0)
		return (1);

bad:
	snprintf(details, sizeof(details),
	    "    wide strings do not match:\n"
	    "           value=\"%ls\"\n"
	    "        expected=\"%ls\"", value, expected);
	return (0);
}

static int
assert_int_equals(int value, int expected)
{
	if (value == expected)
		return (1);

	snprintf(details, sizeof(details),
	    "    ints do not match:\n"
	    "           value=%d\n"
	    "        expected=%d", value, expected);

	return (0);
}

static int
assert_size_t_equals(size_t value, size_t expected)
{
	char cvalue[64], cexpected[64];

	if (value == expected)
		return (1);

	if (value == (size_t)-1) {
		strlcpy(cvalue, "(size_t)-1", sizeof(cvalue));
	} else {
		snprintf(cvalue, sizeof(cvalue), "%lu", value);
	}

	if (expected == (size_t)-1) {
		strlcpy(cexpected, "(size_t)-1", sizeof(cexpected));
	} else {
		snprintf(cexpected, sizeof(cexpected), "%lu", expected);
	}

	snprintf(details, sizeof(details),
	    "    size_t variables do not match:\n"
	    "           value=%s\n"
	    "        expected=%s", cvalue, cexpected);

	return (0);
}

static int
assert_null(const void *p)
{
	if (p == NULL)
		return (1);

	snprintf(details, sizeof(details), "    pointer is not NULL");
	return (0);
}

#include "inc-filelist.c"

int
main(int argc, const char *argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "");

#include "inc-testlist.c"

	printf("%d tests (%d PASS, %d FAIL)\n", tested, passed, failed);

	if (failed) {
		return (1);
	} else {
		return (0);
	}
}
