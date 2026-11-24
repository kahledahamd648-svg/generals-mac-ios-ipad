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

// CnCOnlineHosts.h ///////////////////////////////////////////////////////////
// GeneralsX: C&C:Online (Revora) host redirection for the GameSpy online stack.
//
// The GameSpy SDK is already compiled with GSI_DOMAIN_NAME="server.cnc-online.net"
// (see cmake/gamespy.cmake), but several SDK-generated names (gpsp.*, natneg1-3.*,
// <game>.ms<N>.*, <game>.master.*, <game>.available.*) have no DNS record on
// C&C:Online. CnCOnline_TranslateHost() maps every legacy *.gamespy.com /
// servserv.generals.ea.com name, and every unpublished *.server.cnc-online.net
// name, onto a hostname C&C:Online actually publishes.
//
// This header is plain C so it can be shared by the C GameSpy SDK and the C++ engine.
// Only GameSpy/online code paths use it; LAN and single player never touch it.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h>

#define CNCONLINE_DOMAIN            "server.cnc-online.net"
#define CNCONLINE_GPCM_HOST         "gpcm." CNCONLINE_DOMAIN
#define CNCONLINE_GPSP_HOST         "gpsp." CNCONLINE_DOMAIN
#define CNCONLINE_PEERCHAT_HOST     "peerchat." CNCONLINE_DOMAIN
#define CNCONLINE_MASTER_HOST       "master." CNCONLINE_DOMAIN
#define CNCONLINE_NATNEG1_HOST      "natneg1." CNCONLINE_DOMAIN
#define CNCONLINE_NATNEG2_HOST      "natneg2." CNCONLINE_DOMAIN
#define CNCONLINE_NATNEG3_HOST      "natneg3." CNCONLINE_DOMAIN
#define CNCONLINE_GAMESTATS_HOST    "gamestats." CNCONLINE_DOMAIN
#define CNCONLINE_HTTP_HOST         "http." CNCONLINE_DOMAIN

#ifdef __cplusplus
extern "C" {
#endif

// Translates a GameSpy-era hostname into its C&C:Online equivalent.
// Returns 'out' when a translation was written, or 'host' unchanged when the name
// is not a GameSpy/C&C:Online service name (e.g. LAN addresses, dotted IPs).
const char *CnCOnline_TranslateHost(const char *host, char *out, size_t outSize);

#if !defined(_WIN32)
struct sockaddr;
int CnCOnline_connect(int socket, const struct sockaddr *address, size_t addressLength);
#endif

#ifdef __cplusplus
}
#endif
