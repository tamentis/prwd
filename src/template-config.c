/*
 * Copyright (c) 2015 Bertrand Janin <b@janin.com>
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

#include <stdio.h>
#include <wchar.h>

#include "prwd.h"
#include "template.h"

extern int cfg_hostname;
extern int cfg_mercurial;
extern int cfg_git;
extern int cfg_cleancut;
extern int cfg_uid_indicator;
extern size_t cfg_maxpwdlen;
extern int cfg_newsgroup;
extern wchar_t cfg_filler[MAX_FILLER_LEN];

extern int wopterr;


#define CONCAT(v) do {					\
	vlen = wcslcpy(out, (v), len);			\
	if (vlen > len)					\
		return;					\
	out += vlen;					\
} while (0)


/*
 * build_template_from_config will generate a new template from scratch using
 * the parameters that were gathered from the config file.  This will be used
 * by all legacy users until they transition to the new template format.
 */
void
template_from_config(wchar_t *out, size_t len)
{
	size_t vlen;
	wchar_t buf[64];

	if (cfg_hostname)
		CONCAT(L"${hostname}:");

	if (cfg_mercurial || cfg_git)
		CONCAT(L"${branch}${sep :}");

	if (cfg_cleancut) {
		swprintf(buf, 64, L"${path -c -l %d -f %ls}:", cfg_maxpwdlen,
		    cfg_filler);
	} else if (cfg_newsgroup) {
		swprintf(buf, 64, L"${path -n -l %d -f %ls}:", cfg_maxpwdlen,
		    cfg_filler);
	} else {
		swprintf(buf, 64, L"${path -l %d -f %ls}:", cfg_maxpwdlen,
		    cfg_filler);
	}

	CONCAT(buf);

	if (cfg_uid_indicator)
		CONCAT(L"${uid}");
}
