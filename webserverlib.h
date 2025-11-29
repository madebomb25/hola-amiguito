#ifndef WEBSERVERLIB_H
#define WEBSERVERLIB_H

#include "socket_utils.h"

typedef struct
{
	int socket_fd;
	IPv4Endpoint endpoint;
	char *WEB_ROOT_PATH;
	int max_clients;
} WebServer;

WebServer *start_webserver(const char *web_root_path, int port);
void accept_requests(WebServer *server);
void close_server(WebServer *server);

#endif
