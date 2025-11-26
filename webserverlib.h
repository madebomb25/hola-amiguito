#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "socket_utils.h"

#define ERR_OPEN_SOCKET  -1
#define ERR_BIND_SOCKET  -2
#define ERR_LISTEN       -3

typedef struct sockaddr_in IPv4SocketAddress;

/* TO DO: modificar firma para que el valor que retorna start_webserver no sea int
si no WebServer. Hay que buscar de alguna manera que no pete "WebServer is undefined" */
int start_webserver(int port);
static void end_process_with_error(int error_code);
static int bind_socket_to_endpoint(int socket_fd, IPv4Endpoint *endpoint);
static int start_listening(int socket_fd);
// static void accept_requests(WebServer *server);

#endif
