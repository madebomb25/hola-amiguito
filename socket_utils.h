#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct sockaddr_in IPv4Endpoint;

IPv4Endpoint make_ipv4_endpoint(const char *ip, int port);

#endif