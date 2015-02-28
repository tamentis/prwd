/*
 * Copyright (c) 2015 Bertrand Janin <b@janin.com>
 * Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */
/*-
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron and Thomas Klausner.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code is a rough translation of getopt() from OpenBSD with all the char
 * references upgraded to wchar_t.
 */

#include <wchar.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "wgetopt.h"

int	 wopterr = 1;		/* if error message should be printed */
int	 woptind = 1;		/* index into parent argv vector */
int	 woptopt = '?';		/* character checked for validity */
int	 woptreset;		/* reset getopt */
wchar_t *woptarg;		/* argument associated with option */

#define PRINT_ERROR	((wopterr) && (*options != ':'))

#define FLAG_PERMUTE	0x01	/* permute non-options to the end of argv */
#define FLAG_ALLARGS	0x02	/* treat non-options as args to option "-1" */

/* return values */
#define	BADCH		(int)'?'
#define	BADARG		((*options == ':') ? (int)':' : (int)'?')
#define	INORDER 	(int)1

#define	EMSG		L""

static wchar_t wgetopt_internal(int, wchar_t * const *, const wchar_t *, int);
static int gcd(int, int);
static void permute_args(int, int, int, wchar_t * const *);

static wchar_t *place = EMSG; /* option letter processing */

/* XXX: set woptreset to 1 rather than these two */
static int nonopt_start = -1; /* first non option argument (for permute) */
static int nonopt_end = -1;   /* first option after non options (for permute) */

/* Error messages */
static const char recargchar[] = "option requires an argument -- %c";
static const char recargstring[] = "option requires an argument -- %s";
static const char ambig[] = "ambiguous option -- %.*s";
static const char noarg[] = "option doesn't take an argument -- %.*s";
static const char illoptchar[] = "unknown option -- %c";
static const char illoptstring[] = "unknown option -- %s";

/*
 * Compute the greatest common divisor of a and b.
 */
static int
gcd(int a, int b)
{
	int c;

	c = a % b;
	while (c != 0) {
		a = b;
		b = c;
		c = a % b;
	}

	return (b);
}

/*
 * Exchange the block from nonopt_start to nonopt_end with the block
 * from nonopt_end to opt_end (keeping the same order of arguments
 * in each block).
 */
static void
permute_args(int panonopt_start, int panonopt_end, int opt_end,
	wchar_t * const *nargv)
{
	int cstart, cyclelen, i, j, ncycle, nnonopts, nopts, pos;
	wchar_t *swap;

	/*
	 * compute lengths of blocks and number and size of cycles
	 */
	nnonopts = panonopt_end - panonopt_start;
	nopts = opt_end - panonopt_end;
	ncycle = gcd(nnonopts, nopts);
	cyclelen = (opt_end - panonopt_start) / ncycle;

	for (i = 0; i < ncycle; i++) {
		cstart = panonopt_end+i;
		pos = cstart;
		for (j = 0; j < cyclelen; j++) {
			if (pos >= panonopt_end)
				pos -= nnonopts;
			else
				pos += nopts;
			swap = nargv[pos];
			/* LINTED const cast */
			((wchar_t **)nargv)[pos] = nargv[cstart];
			/* LINTED const cast */
			((wchar_t **)nargv)[cstart] = swap;
		}
	}
}

/*
 * wgetopt_internal --
 *	Parse argc/argv argument vector.  Called by user level routines.
 */
static wchar_t
wgetopt_internal(int nargc, wchar_t * const *nargv, const wchar_t *options,
    int flags)
{
	wchar_t *oli;				/* option letter list index */
	int optchar;
	static int posixly_correct = -1;

	if (options == NULL)
		return (-1);

	/*
	 * XXX Some GNU programs (like cvs) set woptind to 0 instead of
	 * XXX using woptreset.  Work around this braindamage.
	 */
	if (woptind == 0)
		woptind = woptreset = 1;

	/*
	 * Disable GNU extensions if POSIXLY_CORRECT is set or options
	 * string begins with a '+'.
	 */
	if (posixly_correct == -1 || woptreset)
		posixly_correct = (getenv("POSIXLY_CORRECT") != NULL);
	if (*options == L'-')
		flags |= FLAG_ALLARGS;
	else if (posixly_correct || *options == L'+')
		flags &= ~FLAG_PERMUTE;
	if (*options == L'+' || *options == L'-')
		options++;

	woptarg = NULL;
	if (woptreset)
		nonopt_start = nonopt_end = -1;
start:
	if (woptreset || !*place) {		/* update scanning pointer */
		woptreset = 0;
		if (woptind >= nargc) {          /* end of argument vector */
			place = EMSG;
			if (nonopt_end != -1) {
				/* do permutation, if we have to */
				permute_args(nonopt_start, nonopt_end,
				    woptind, nargv);
				woptind -= nonopt_end - nonopt_start;
			}
			else if (nonopt_start != -1) {
				/*
				 * If we skipped non-options, set woptind
				 * to the first of them.
				 */
				woptind = nonopt_start;
			}
			nonopt_start = nonopt_end = -1;
			return (-1);
		}
		if (*(place = nargv[woptind]) != L'-' ||
		    (place[1] == L'\0' && wcschr(options, L'-') == NULL)) {
			place = EMSG;		/* found non-option */
			if (flags & FLAG_ALLARGS) {
				/*
				 * GNU extension:
				 * return non-option as argument to option 1
				 */
				woptarg = nargv[woptind++];
				return (INORDER);
			}
			if (!(flags & FLAG_PERMUTE)) {
				/*
				 * If no permutation wanted, stop parsing
				 * at first non-option.
				 */
				return (-1);
			}
			/* do permutation */
			if (nonopt_start == -1)
				nonopt_start = woptind;
			else if (nonopt_end != -1) {
				permute_args(nonopt_start, nonopt_end,
				    woptind, nargv);
				nonopt_start = woptind -
				    (nonopt_end - nonopt_start);
				nonopt_end = -1;
			}
			woptind++;
			/* process next argument */
			goto start;
		}
		if (nonopt_start != -1 && nonopt_end == -1)
			nonopt_end = woptind;

		/*
		 * If we have "-" do nothing, if "--" we are done.
		 */
		if (place[1] != L'\0' && *++place == L'-' && place[1] == L'\0') {
			woptind++;
			place = EMSG;
			/*
			 * We found an option (--), so if we skipped
			 * non-options, we have to permute.
			 */
			if (nonopt_end != -1) {
				permute_args(nonopt_start, nonopt_end,
				    woptind, nargv);
				woptind -= nonopt_end - nonopt_start;
			}
			nonopt_start = nonopt_end = -1;
			return (-1);
		}
	}

	if ((optchar = (int)*place++) == (int)':' ||
	    (optchar == (int)'-' && *place != '\0') ||
	    (oli = wcschr(options, optchar)) == NULL) {
		/*
		 * If the user specified "-" and  '-' isn't listed in
		 * options, return -1 (non-option) as per POSIX.
		 * Otherwise, it is an unknown option character (or ':').
		 */
		if (optchar == (int)'-' && *place == L'\0')
			return (-1);
		if (!*place)
			++woptind;
		if (PRINT_ERROR)
			warnx(illoptchar, optchar);
		woptopt = optchar;
		return (BADCH);
	}
	if (*++oli != L':') {			/* doesn't take argument */
		if (!*place)
			++woptind;
	} else {				/* takes (optional) argument */
		woptarg = NULL;
		if (*place)			/* no white space */
			woptarg = place;
		else if (oli[1] != L':') {	/* arg not optional */
			if (++woptind >= nargc) {	/* no arg */
				place = EMSG;
				if (PRINT_ERROR)
					warnx(recargchar, optchar);
				woptopt = optchar;
				return (BADARG);
			} else
				woptarg = nargv[woptind];
		}
		place = EMSG;
		++woptind;
	}
	/* dump back option letter */
	return (optchar);
}

/*
 * getopt --
 *	Parse argc/argv argument vector.
 *
 * [eventually this will replace the BSD getopt]
 */
wchar_t
wgetopt(int nargc, wchar_t * const *nargv, const wchar_t *options)
{

	/*
	 * We don't pass FLAG_PERMUTE to wgetopt_internal() since
	 * the BSD getopt(3) (unlike GNU) has never done this.
	 *
	 * Furthermore, since many privileged programs call getopt()
	 * before dropping privileges it makes sense to keep things
	 * as simple (and bug-free) as possible.
	 */
	return (wgetopt_internal(nargc, nargv, options, 0));
}
