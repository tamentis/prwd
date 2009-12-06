/* $Id$ */
/*
 * Copyright (c) 2009 Bertrand Janin <tamentis@neopulsar.org>
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
#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <err.h>

#define FILLER_LEN	16	// maximum filler length
#define FILLER_DEF	"..."	// default filler
#define MAXPWD_LEN	24	// default maximum length
#define MAX_ALIASES	64	// maximum number of aliases
#define ALIAS_NAME_LEN	32	// size of an alias

#define WHITESPACE	" \t\r\n"
#define QUOTE		"\""

int	 cfg_maxpwdlen = MAXPWD_LEN;
int 	 cfg_cleancut = 0;
int	 cfg_newsgroup = 0;
char	 cfg_filler[FILLER_LEN] = FILLER_DEF;
char	*home;
int	 alias_count = 0;

struct {
	char	name[ALIAS_NAME_LEN];
	char	path[MAXPATHLEN];
} aliases[MAX_ALIASES];


#ifndef HAVE_STRTONUM
/**
 * Copyright (c) 2004 Ted Unangst and Todd Miller
 * All rights reserved.
 */
#define INVALID 	1
#define TOOSMALL 	2
#define TOOLARGE 	3
long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp)
{
	long long ll = 0;
	char *ep;
	int error = 0;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval)
		error = INVALID;
	else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
#else /* ELSE HAVE_STRTONUM */
long long strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);
#endif /* HAVE_STRTONUM */


#ifndef HAVE_STRLCPY
/* 
 * This function, extracted from OpenBSD is under ISC license and is
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}
#else /* ELSE HAVE_STRLCPY */
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif /* !HAVE_STRLCPY */


/**
 * Return next token in configuration line, one word at a time.
 * This is borrowed from OpenSSH, under ISC license,
 *
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 * Copyright (c) 2005,2006 Damien Miller.  All rights reserved.
 */
char *
strdelim(char **s)
{
	char *old;
	int wspace = 0;

	if (*s == NULL)
		return NULL;

	old = *s;

	*s = strpbrk(*s, WHITESPACE QUOTE "=");
	if (*s == NULL)
		return (old);

	if (*s[0] == '\"') {
		memmove(*s, *s + 1, strlen(*s)); /* move nul too */
		/* Find matching quote */
		if ((*s = strpbrk(*s, QUOTE)) == NULL) {
			return (NULL);		/* no matching quote */
		} else {
			*s[0] = '\0';
			return (old);
		}
	}

	/* Allow only one '=' to be skipped */
	if (*s[0] == '=')
		wspace = 1;
	*s[0] = '\0';

	/* Skip any extra whitespace after first token */
	*s += strspn(*s + 1, WHITESPACE) + 1;
	if (*s[0] == '=' && !wspace)
		*s += strspn(*s + 1, WHITESPACE) + 1;

	return (old);
}


/**
 * Panic exit, preferably running in walls and waving your arms over your head.
 */
void
fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(-1);
}


/**
 * Add a new alias to the list.
 */
void
add_alias(char *name, char *value, int linenum)
{
	if (strlen(value) < strlen(name))
		fatal("prwd: alias name should not be longer than the value.\n");
	if (strchr(name, '/') != NULL)
		fatal("prwd: alias name should not contain any '/' (slash).\n");
	strlcpy(aliases[alias_count].name, name, ALIAS_NAME_LEN);
	strlcpy(aliases[alias_count].path, value, MAXPATHLEN);
	alias_count++;
}


/**
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
void
set_variable(char *name, char *value, int linenum)
{
	const char *errstr = NULL;

	/* set maxlength <int> */
	if (strcasecmp(name, "maxlength") == 0) {
		if (value == NULL || *value == '\0') {
			cfg_maxpwdlen = 0;
			return;
		}
		cfg_maxpwdlen = strtonum(value, 0, 256, &errstr);
		if (errstr != NULL)
			fatal("prwd: invalid number for set maxlength.\n");

	/* set filler <string> */
	} else if (strcasecmp(name, "filler") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_filler = '\0';
			return;
		}
		strlcpy(cfg_filler, value, FILLER_LEN);

	/* set cleancut <bool> */
	} else if (strcasecmp(name, "cleancut") == 0) {
		cfg_cleancut = (value != NULL && *value == 'o') ? 1 : 0;

	/* set newsgroup <bool> */
	} else if (strcasecmp(name, "newsgroup") == 0) {
		cfg_newsgroup = (value != NULL && *value == 'o') ? 1 : 0;

	/* ??? */
	} else {
		fatal("prwd: unknown varible for set on line %d.\n", linenum);
	}
}


/**
 * Parse a single line of the config file. Returns 0 on success or
 * anything else if an error occured, it will be rare since most
 * fatal errors will quit the program with an error message
 * anyways.
 */
int
process_config_line(char *line, int linenum)
{
	int len;
	char *keyword, *name, *value;

        /* Strip trailing whitespace */
	for (len = strlen(line) - 1; len > 0; len--) {
		if (strchr(WHITESPACE, line[len]) == NULL)
			break;
		line[len] = '\0';
	}

	/* Get the keyword. (Each line is supposed to begin with a keyword). */
	if ((keyword = strdelim(&line)) == NULL)
		return 0;

	/* Ignore leading whitespace. */
	if (*keyword == '\0')
		keyword = strdelim(&line);

	if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
		return 0;

	/* set varname value */
	if (strcasecmp(keyword, "set") == 0) {
		if ((name = strdelim(&line)) == NULL)
			fatal("prwd: set without variable name on line %d.\n", linenum);
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* alias short long */
	} else if (strcmp(keyword, "alias") == 0) {
		if ((name = strdelim(&line)) == NULL)
			fatal("prwd: alias without name on line %d.\n", linenum);
		value = strdelim(&line);
		add_alias(name, value, linenum);

	/* Unknown operation... God help us. */
	} else {
		fatal("prwd: unknown command on line %d.\n", linenum);
	}

	return 0;
}


/**
 * Open the file and feed each line one by one to process_config_line.
 */
void
read_config()
{
	FILE *fp;
	char line[128];
	int linenum = 1;
	char path[MAXPATHLEN];

	snprintf(path, MAXPATHLEN, "%s/.prwdrc", home);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp))
		process_config_line(line, linenum++);

	fclose(fp);
}


/**
 * Take a string and replace all the full words by their first
 * letters, except the last one.
 */
void
newsgroupize(char *s)
{
	char t[MAXPATHLEN];
	char *last, *org = s;
	int idx = 0;

	/* Unless we are starting from a / (slash), we can use the first one */
	if (*s != '/')
		t[idx++] = *s;
	t[idx++] = '/';

	/* For every component, add the first letter and a slash */
	while ((s = strchr(s, '/')) != NULL) {
		last = ++s;
		t[idx++] = (char)*s;
		t[idx++] = '/';
	}
	
	/* Copy the letters+slash and make sure the last part is left untouched. */
	strlcpy(org, t, idx);
	strlcpy(org + idx - 2, last, strlen(last) + 1);
}


int
main(int ac, const char **av)
{
	size_t len, nlen;
	int i;
	char *pwd, *s, *last;

	if (ac != 1) {
		printf("usage: prwd\n");
		exit(-1);
	}

	home = getenv("HOME");
	if (home == NULL || *home == '\0')
		errx(0, "Unknown variable '$HOME'.");

	pwd = getcwd(NULL, MAXPATHLEN);
	if (pwd == NULL)
		errx(0, "Unable to get current working directory.");
	s = pwd;

	read_config();

	/* Alias handling */
	for (i = 0; i < alias_count; i++) {
		len = strlen(aliases[i].path);
		if (strncmp(aliases[i].path, pwd, len) == 0) {
			nlen = strlen(aliases[i].name);
			s += (len - nlen);
			strncpy(s, aliases[i].name, nlen);
		}
	}

	/* Replace the beginning with ~ for directories within $HOME. */
	len = strlen(home);
	if (strncmp(home, s, len) == 0) {
		s += (len - 1);
		*s = '~';
	}

	/* Newsgroup mode, keep only the first letters. */
	if (cfg_newsgroup == 1)
		newsgroupize(s);

	/* If the path is still too long, crop it. */
	len = strlen(s);
	if (cfg_maxpwdlen > 0 && len > cfg_maxpwdlen) {
		if (cfg_cleancut == 1 && cfg_newsgroup != 1) {
			len = strlen(cfg_filler);
			while (strlen(s) > (cfg_maxpwdlen - len)) {
				s++;
				s = strchr(s, '/');
				if (s == NULL)
					break;
				last = s;
			}
			if (s == NULL)
				s = last;
			else {
				s -= len;
				strncpy(s, cfg_filler, len);
			}
		} else {
			s += (len - cfg_maxpwdlen);
			strncpy(s, cfg_filler, strlen(cfg_filler));
		}
	}

	printf("%s\n", s);
	free(pwd);

	return 0;
}
