#ifndef WEBSERVERLIB_H
#define WEBSERVERLIB_H

#include "socket_utils.h"
#include "http_utils.h"

#define HTTP_OK 200
#define HTTP_NOTFOUND 404
#define MAX_PATH_LENGTH 1024
#define BUFFER_SIZE 4096

typedef struct WebServer WebServer;
typedef struct HttpRequest HttpRequest;

WebServer *start_webserver(char *WEB_ROOT_PATH, char *DEFAULT_WEB_PATH, int port);
void accept_requests(WebServer *server);
void close_server(WebServer *server);

void parse_http_request(const char *raw_request, HttpRequest **request);
int http_request_has_valid_structure(HttpRequest *request);
int http_request_is_GET(HttpRequest *request);

#endif
