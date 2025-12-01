#ifndef WEBSERVERLIB_H
#define WEBSERVERLIB_H

#include "socket_utils.h"
#include "http_utils.h"

#define HTTP_OK 200
#define HTTP_NOTFOUND 404

typedef struct WebServer WebServer;

WebServer* start_webserver(char* WEB_ROOT_PATH, char* DEFAULT_WEB_PATH, int port);
void close_server(WebServer* server);

#endif

