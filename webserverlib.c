#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include "socket_utils.h"

#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

typedef struct 
{
	int socket_fd;
	IPv4Endpoint endpoint;
	int max_clients;
	
} WebServer;

WebServer *start_webserver(int port)
{	
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) 
		end_process_with_error(ERR_OPEN_SOCKET);
    
    IPv4Endpoint endpoint;
    endpoint = make_ipv4_endpoint("127.0.0.0", port);
    int is_binded = bind_socket_to_endpoint(socket_fd, &endpoint);
    if (!is_binded)
        end_process_with_error(ERR_BIND_SOCKET);
    
    int is_listening = start_listening(socket_fd);
    if (!is_listening)
        end_process_with_error(ERR_LISTEN);
    
    WebServer *server = malloc(sizeof(WebServer));
    if (!server)
    {
		free(server);
		end_process_with_error(ERR_MEMORY);
	}
	// Asignación de valores a los campos del struct 
	server->endpoint = endpoint;
	server->socket_fd = socket_fd;
	
    return server;
}

static void end_process_with_error(int error_code)
{
	if (error_code == ERR_OPEN_SOCKET)
		perror("ERR: No se ha podido generar un socket.");
	else if (error_code == ERR_BIND_SOCKET)
		perror("ERR: No se ha podido asignar un endpoint a un socket en un servidor web");
	else if (error_code == ERR_LISTEN)
		perror("ERR: No se ha podido comenzar a escuchar a un socket en un servidor web.");
	else if (error_code == ERR_MEMORY)
		perror("ERR: No se ha podido reservar memoria para un servidor web.");
	else
		perror("ERR: Ha ocurrido un error desconocido al abrir un servidor web.");
	exit(error_code);
}

static int bind_socket_to_endpoint(int socket_fd, IPv4Endpoint *endpoint)
{
    return bind(socket_fd, (struct sockaddr *)endpoint, sizeof(*endpoint)) == 0;
}

static int start_listening(int socket_fd)
{
    return listen(socket_fd, MAX_CLIENTS_IN_QUEUE) == 0;
}

static void accept_requests(WebServer *server) 
{
	int server_fd = server->socket_fd;

	while(*server)
	{
		IPv4Endpoint client_addr;
		socklen_t client_addr_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr);

		if (client_fd >= 0)
		{
			handle_request(client_fd);
		}
		else
		{
			/*Esto se podría implementar mediante end_process_with_error 
			pero en caso de que una petición falle no queremos abortar la ejecución
			del servidor, si no que intente aceptar el resto de peticiones entrantes. */
			perror("ERR: No se ha podido aceptar una petición.");
		}
	}
}

static void end_server(WebServer *server)
{
	close(server->socket_fd);
	free(server);
}
