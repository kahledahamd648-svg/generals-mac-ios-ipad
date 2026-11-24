#include "cnc_dns_override.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#undef gethostbyname
#undef getaddrinfo
#undef connect

#ifndef _WIN32

static void cnc_copy_lower(char *output, size_t outputSize, const char *input)
{
	size_t index;
	size_t length;

	if (output == NULL || outputSize == 0 || input == NULL)
		return;

	length = strlen(input);
	if (length >= outputSize)
		length = outputSize - 1;

	for (index = 0; index < length; ++index)
		output[index] = (char)tolower((unsigned char)input[index]);
	output[length] = '\0';
}

void cnc_dns_log(const char *message)
{
	FILE *logFile;
	const char *home;
	char path[1024];

	if (message == NULL)
		return;

	fprintf(stderr, "[CNCOnline] %s\n", message);

#if defined(__APPLE__)
	home = getenv("HOME");
	if (home != NULL && home[0] != '\0')
	{
		snprintf(path, sizeof(path), "%s/Documents/cnc_online_net.log", home);
		logFile = fopen(path, "a");
		if (logFile != NULL)
		{
			fprintf(logFile, "[CNCOnline] %s\n", message);
			fclose(logFile);
		}
	}
#endif
}

static int cnc_is_local_or_broadcast_address(const char *host)
{
	struct in_addr address;
	uint32_t numericAddress;

	if (host == NULL || inet_pton(AF_INET, host, &address) != 1)
		return 0;

	numericAddress = ntohl(address.s_addr);
	if ((numericAddress >> 24) == 127)
		return 1;
	if (numericAddress == 0 || numericAddress == 0xFFFFFFFF)
		return 1;
	if ((numericAddress >> 24) == 10)
		return 1;
	if ((numericAddress >> 24) == 172 &&
		((numericAddress >> 16) & 0xFF) >= 16 &&
		((numericAddress >> 16) & 0xFF) <= 31)
		return 1;
	if ((numericAddress >> 24) == 192 && ((numericAddress >> 16) & 0xFF) == 168)
		return 1;

	return 0;
}

const char *cnc_dns_translate_host(const char *host, char *buffer, size_t bufferSize)
{
	char lower[256];
	char message[512];
	size_t length;

	if (host == NULL || buffer == NULL || bufferSize == 0)
		return host;

	cnc_copy_lower(lower, sizeof(lower), host);
	length = strlen(lower);
	if (length > 0 && lower[length - 1] == '.')
		lower[--length] = '\0';

	if (cnc_is_local_or_broadcast_address(lower))
	{
		snprintf(message, sizeof(message), "preserve local or broadcast address %s", host);
		cnc_dns_log(message);
		snprintf(buffer, bufferSize, "%s", host);
		return buffer;
	}

	if (strstr(lower, "available.gamespy.com") != NULL ||
		strstr(lower, "available.server.cnc-online.net") != NULL ||
		strstr(lower, "available.") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_AVAILABLE);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "gpcm.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_GPCM);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "gpsp.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_GPSP);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "peerchat.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_PEERCHAT);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "master.gamespy.com") != NULL ||
		strstr(lower, ".ms") != NULL && strstr(lower, ".gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_MASTER);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "gamestats.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_GAMESTATS);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "natneg1.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_NATNEG1);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "natneg2.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_NATNEG2);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, "natneg3.gamespy.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_NATNEG3);
		snprintf(message, sizeof(message), "DNS redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}
	if (strstr(lower, ".gamespy.com") != NULL ||
		strstr(lower, ".gamespyarcade.com") != NULL)
	{
		snprintf(buffer, bufferSize, "%s", CNC_DNS_DOMAIN);
		snprintf(message, sizeof(message), "DNS fallback redirect %s -> %s", host, buffer);
		cnc_dns_log(message);
		return buffer;
	}

	return host;
}

struct hostent *cnc_gethostbyname(const char *name)
{
	char translated[256];
	char message[512];
	const char *resolvedName;
	struct hostent *host;

	resolvedName = cnc_dns_translate_host(name, translated, sizeof(translated));
	host = gethostbyname(resolvedName);
	snprintf(message, sizeof(message), "gethostbyname %s resolved=%s",
		name == NULL ? "(null)" : name, host != NULL ? "success" : "failure");
	cnc_dns_log(message);
	return host;
}

int cnc_getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **result)
{
	char translated[256];
	char message[512];
	const char *resolvedNode;
	int returnCode;

	resolvedNode = cnc_dns_translate_host(node, translated, sizeof(translated));
	returnCode = getaddrinfo(resolvedNode, service, hints, result);
	snprintf(message, sizeof(message), "getaddrinfo %s -> %s return=%d",
		node == NULL ? "(null)" : node,
		resolvedNode == NULL ? "(null)" : resolvedNode,
		returnCode);
	cnc_dns_log(message);
	return returnCode;
}

int cnc_connect(int socketDescriptor, const struct sockaddr *address, socklen_t addressLength)
{
	char message[512];
	int returnCode;

	returnCode = connect(socketDescriptor, address, addressLength);
	if (address != NULL && address->sa_family == AF_INET)
	{
		const struct sockaddr_in *addressIn = (const struct sockaddr_in *)address;
		char addressText[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addressIn->sin_addr, addressText, sizeof(addressText));
		snprintf(message, sizeof(message), "connect %s:%u return=%d",
			addressText, ntohs(addressIn->sin_port), returnCode);
	}
	else
	{
		snprintf(message, sizeof(message), "connect return=%d", returnCode);
	}
	cnc_dns_log(message);
	return returnCode;
}

#endif
