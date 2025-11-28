#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include "socket_utils.h"

#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024

typedef struct 
{
	int socket_fd;
	IPv4Endpoint endpoint;
	char *WEB_ROOT_PATH;
	int max_clients;
	
} WebServer;

WebServer *start_webserver(char* WEB_ROOT_PATH, int port)
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

	server->endpoint = endpoint;
	server->socket_fd = socket_fd;
	server->WEB_ROOT_PATH = WEB_ROOT_PATH;
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

	while(server)
	{
		IPv4Endpoint client_addr;
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr);

		if (client_fd >= 0)
			handle_request(server, client_fd);
			
		else
			/*Esto se podría implementar mediante end_process_with_error 
			pero si falla no queremos abortar el servidor. Pueden haber
			peticiones entrantes. */
			perror("ERR: No se ha podido aceptar una petición.");
	}
}

static void handle_request(WebServer *server, int client_fd) {
	char request_buffer[BUFFER_SIZE]; 
	int bytes_received = read(client_fd, request_buffer, BUFFER_SIZE - 1);
	
	char *method;
	if (bytes_received < 0)
		perror("ERR: No se ha podido manejar una petición HTTP.\n");

	else if (bytes_received == 0)
		printf("El cliente terminó la conexión.\n");

	else {
		request_buffer[bytes_received] = '\0';
		char *requested_path;
		// Truncamos la primera línea de la petición
		char *request_line = strtok(request_buffer, "\r\n");
		
		if (request_line == NULL)
			send_404_response(client_fd);
			
		else {
			method = strtok(request_line, " ");
			requested_path = strtok(NULL, " ");
		}

		if (method != NULL && path != NULL && strncmp(method, "GET", 3) == 0) {
			send_files(client_fd, server->WEB_ROOT_PATH, requested_path, 200);
		}
		else {
			send_404_response(client_fd);
		}
	}
	close(client_fd);
}

void close_server(WebServer *server)
{
	close(server->socket_fd);
	free(server);
}
