#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include "socket_utils.h"

typedef struct sockaddr_in IPv4Endpoint;

IPv4Endpoint make_ipv4_endpoint(char* ip, int port)
{
    IPv4Endpoint endpoint = {0};
    endpoint.sin_family = AF_INET;

    if (strcmp(ip, "0.0.0.0") == 0) 
        endpoint.sin_addr.s_addr = htonl(INADDR_ANY);
    else 
        endpoint.sin_addr.s_addr = inet_addr(ip);

    endpoint.sin_port = htons(port);
    return endpoint;
}
