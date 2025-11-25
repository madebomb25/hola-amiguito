#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <netinet/in.h>

#define ERR_OPEN_SOCKET  -1
#define ERR_BIND_SOCKET  -2
#define ERR_LISTEN       -3

typedef struct sockaddr_in IPv4SocketAddress;

int start_webserver(int port);

#endif
