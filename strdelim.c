/* $Id$ */
/* $OpenBSD: misc.c,v 1.69 2008/06/13 01:38:23 dtucker Exp $ */
/*
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 * Copyright (c) 2005,2006 Damien Miller.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <wchar.h>


/* Characters considered whitespace in strsep calls. */
#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""

/* return next token in configuration line */
wchar_t *
strdelim(wchar_t **s)
{
	wchar_t *old;
	int wspace = 0;

	if (*s == NULL)
		return NULL;

	old = *s;

	*s = wcspbrk(*s, WHITESPACE QUOTE "=");
	if (*s == NULL)
		return (old);

	if (*s[0] == L'\"') {
		wmemmove(*s, *s + 1, wcslen(*s)); /* move nul too */
		/* Find matching quote */
		if ((*s = wcspbrk(*s, QUOTE)) == NULL) {
			return (NULL);		/* no matching quote */
		} else {
			*s[0] = L'\0';
			return (old);
		}
	}

	/* Allow only one '=' to be skipped */
	if (*s[0] == L'=')
		wspace = 1;
	*s[0] = L'\0';

	/* Skip any extra whitespace after first token */
	*s += wcsspn(*s + 1, WHITESPACE) + 1;
	if (*s[0] == L'=' && !wspace)
		*s += wcsspn(*s + 1, WHITESPACE) + 1;

	return (old);
}

