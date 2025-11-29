#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "socket_utils.h"

IPv4Endpoint make_ipv4_endpoint(const char *ip, int port)
{
    IPv4Endpoint endpoint;
    memset(&endpoint, 0, sizeof(endpoint));
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = inet_addr(ip);
    endpoint.sin_port = htons(port);
    return endpoint;
}