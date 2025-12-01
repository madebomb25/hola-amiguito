#ifndef WEBSERVER_LIB_H
#define WEBSERVER_LIB_H

#include "socket_utils.h" 
#include <stddef.h> 
#include <stdlib.h>
#include <unistd.h>


#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024
#define WEB_ROOT "www"


typedef struct 
{
    int socket_fd;
    IPv4Endpoint endpoint;
    int max_clients;
} WebServer;


WebServer *start_webserver(int port);

void accept_requests(WebServer *server);

void send_files(int client_fd, char *path, int status_code);

void send_404_response(int client_fd);

void send_static_404_response(int client_fd);

void end_server(WebServer *server); 

#endif 