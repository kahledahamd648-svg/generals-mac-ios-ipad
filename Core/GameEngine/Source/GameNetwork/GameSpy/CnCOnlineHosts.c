/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// CnCOnlineHosts.c ///////////////////////////////////////////////////////////
// GeneralsX: C&C:Online (Revora) host redirection for the GameSpy SDK.
//
// This file is compiled into the GameSpy SDK library target (see cmake/gamespy.cmake).
// On non-Windows platforms the SDK modules are built with
//   gethostbyname=CnCOnline_gethostbyname  getaddrinfo=CnCOnline_getaddrinfo
// so every DNS lookup the SDK performs (GP, peer/chat, qr2, server browsing,
// natneg, ghttp, gstats) passes through CnCOnline_TranslateHost() first.
// This file itself is compiled WITHOUT those definitions, so it calls the real
// resolver functions.
//
// Published C&C:Online records (all point at the same backend):
//   gpcm / peerchat / master / natneg / gamestats / http / login .server.cnc-online.net
// Unpublished names the SDK would otherwise generate, and their mapping:
//   gpsp.*                -> gpcm.server.cnc-online.net
//   natneg1|2|3.*         -> natneg.server.cnc-online.net
//   <game>.ms<N>.*        -> master.server.cnc-online.net
//   <game>.master.*       -> master.server.cnc-online.net
//   <game>.available.*    -> master.server.cnc-online.net
//   servserv.generals.ea.com -> http.server.cnc-online.net
///////////////////////////////////////////////////////////////////////////////

#include "CnCOnlineHosts.h"

#include <string.h>
#include <ctype.h>

#define CNC_GAMESPY_SUFFIX    ".gamespy.com"
#define CNC_CNCONLINE_SUFFIX  "." CNCONLINE_DOMAIN
#define CNC_SERVSERV_HOST     "servserv.generals.ea.com"
#define CNC_MAX_HOST          256

static int cnc_label_equals(const char *label, size_t len, const char *word)
{
	return strlen(word) == len && strncmp(label, word, len) == 0;
}

// True if label is <prefix> followed by one or more decimal digits (e.g. "ms19", "natneg2").
static int cnc_label_prefix_digits(const char *label, size_t len, const char *prefix)
{
	size_t plen = strlen(prefix);
	size_t i;
	if (len <= plen || strncmp(label, prefix, plen) != 0)
		return 0;
	for (i = plen; i < len; ++i)
	{
		if (!isdigit((unsigned char)label[i]))
			return 0;
	}
	return 1;
}

// Returns the published C&C:Online service label for a GameSpy service prefix,
// or NULL when the prefix is not a known service.
static const char *cnc_service_for(const char *prefix, size_t len)
{
	const char *label = prefix;
	size_t llen = len;
	size_t i;

	// The service is identified by the last label of the prefix
	// ("ccgenzh.ms19" -> "ms19", "natneg1" -> "natneg1").
	for (i = len; i > 0; --i)
	{
		if (prefix[i - 1] == '.')
		{
			label = prefix + i;
			llen = len - i;
			break;
		}
	}

	if (cnc_label_equals(label, llen, "gpcm") || cnc_label_equals(label, llen, "gpsp"))
		return "gpcm";
	if (cnc_label_equals(label, llen, "peerchat"))
		return "peerchat";
	if (cnc_label_equals(label, llen, "natneg") || cnc_label_prefix_digits(label, llen, "natneg"))
		return "natneg";
	if (cnc_label_equals(label, llen, "master") || cnc_label_equals(label, llen, "available")
		|| cnc_label_prefix_digits(label, llen, "ms"))
		return "master";
	if (cnc_label_equals(label, llen, "gamestats"))
		return "gamestats";

	return NULL; // e.g. http/login: already published, or unknown service
}

static int cnc_write_host(char *out, size_t outSize, const char *a, size_t alen, const char *b)
{
	size_t blen = strlen(b);
	if (alen + blen + 1 > outSize)
		return 0;
	memcpy(out, a, alen);
	memcpy(out + alen, b, blen + 1);
	return 1;
}

const char *CnCOnline_TranslateHost(const char *host, char *out, size_t outSize)
{
	char lower[CNC_MAX_HOST];
	size_t len;
	size_t i;
	size_t gsLen = sizeof(CNC_GAMESPY_SUFFIX) - 1;
	size_t cncLen = sizeof(CNC_CNCONLINE_SUFFIX) - 1;
	int fromGameSpy;
	const char *service;

	if (host == NULL || out == NULL || outSize == 0)
		return host;

	len = strlen(host);
	if (len == 0 || len >= sizeof(lower))
		return host;

	for (i = 0; i < len; ++i)
		lower[i] = (char)tolower((unsigned char)host[i]);
	lower[len] = '\0';

	// Tolerate a fully-qualified trailing dot.
	if (lower[len - 1] == '.')
		lower[--len] = '\0';

	if (strcmp(lower, CNC_SERVSERV_HOST) == 0)
	{
		return cnc_write_host(out, outSize, CNCONLINE_HTTP_HOST, strlen(CNCONLINE_HTTP_HOST), "") ? out : host;
	}

	if (len > gsLen && strcmp(lower + len - gsLen, CNC_GAMESPY_SUFFIX) == 0)
	{
		fromGameSpy = 1;
		len -= gsLen;
	}
	else if (len > cncLen && strcmp(lower + len - cncLen, CNC_CNCONLINE_SUFFIX) == 0)
	{
		fromGameSpy = 0;
		len -= cncLen;
	}
	else
	{
		return host; // not a GameSpy / C&C:Online service name (LAN, IP literal, ...)
	}

	service = cnc_service_for(lower, len);
	if (service != NULL)
	{
		return cnc_write_host(out, outSize, service, strlen(service), CNC_CNCONLINE_SUFFIX) ? out : host;
	}

	if (fromGameSpy)
	{
		// Unknown GameSpy service: <prefix>.gamespy.com -> <prefix>.server.cnc-online.net
		return cnc_write_host(out, outSize, lower, len, CNC_CNCONLINE_SUFFIX) ? out : host;
	}

	return host; // already a C&C:Online name we have no special mapping for
}

///////////////////////////////////////////////////////////////////////////////
// Resolver wrappers used by the GameSpy SDK on non-Windows platforms.
///////////////////////////////////////////////////////////////////////////////
#if !defined(_WIN32)

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct hostent *CnCOnline_gethostbyname(const char *name);
int CnCOnline_getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **res);

struct hostent *CnCOnline_gethostbyname(const char *name)
{
	char buffer[CNC_MAX_HOST];
	return gethostbyname(CnCOnline_TranslateHost(name, buffer, sizeof(buffer)));
}

int CnCOnline_getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **res)
{
	char buffer[CNC_MAX_HOST];
	return getaddrinfo(CnCOnline_TranslateHost(node, buffer, sizeof(buffer)), service, hints, res);
}

#endif // !_WIN32
