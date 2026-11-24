#ifndef CNC_DNS_OVERRIDE_H
#define CNC_DNS_OVERRIDE_H

#if !defined(_WIN32)

#include <netdb.h>
#include <sys/socket.h>

#define CNC_DNS_DOMAIN "server.cnc-online.net"
#define CNC_DNS_GPCM "gpcm.server.cnc-online.net"
#define CNC_DNS_GPSP "gpsp.server.cnc-online.net"
#define CNC_DNS_PEERCHAT "peerchat.server.cnc-online.net"
#define CNC_DNS_MASTER "master.server.cnc-online.net"
#define CNC_DNS_AVAILABLE "master.server.cnc-online.net"
#define CNC_DNS_NATNEG1 "natneg1.server.cnc-online.net"
#define CNC_DNS_NATNEG2 "natneg2.server.cnc-online.net"
#define CNC_DNS_NATNEG3 "natneg3.server.cnc-online.net"
#define CNC_DNS_GAMESTATS "gamestats.server.cnc-online.net"

#ifdef __cplusplus
extern "C" {
#endif

struct hostent *cnc_gethostbyname(const char *name);
int cnc_getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **result);
int cnc_connect(int socketDescriptor, const struct sockaddr *address, socklen_t addressLength);
void cnc_dns_log(const char *message);
const char *cnc_dns_translate_host(const char *host, char *buffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif

#ifndef CNC_DNS_OVERRIDE_NO_MACROS
#define gethostbyname cnc_gethostbyname
#define getaddrinfo cnc_getaddrinfo
#define connect cnc_connect
#endif

#endif

#endif
