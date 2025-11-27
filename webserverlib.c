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
#define WEB_ROOT "www"

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

static const char *get_mime_type(const char *path) {
	const char *file_extension = strrchr(path, '.');
	const char DEFAULT_RESPONSE = "application/octet-stream";

	if (!file_extension) return DEFAULT_RESPONSE;

	if (strcmp(file_extension, ".html") == 0)
		return "text/html";
		
    else if (strcmp(file_extension, ".css") == 0)
		return "text/css";
		
    else if (strcmp(file_extension, ".js") == 0)
		return "application/javascript";
		
    else if (strcmp(file_extension, ".jpg") == 0 || 
			 strcmp(file_extension, ".jpeg") == 0)
		return "image/jpeg";
		
    else if (strcmp(file_extension, ".png") == 0)
		return "image/png";
		
    else if (strcmp(file_extension, ".gif") == 0)
		return "image/gif";
		
	else
		return DEFAULT_RESPONSE;
}

void send_files(int client_fd, char *path, int status_code) {
	char local_path[MAX_PATH_LENGTH + 5]; 
	FILE *file = NULL; 
	long file_size = 0;
	char header_buffer[512]; 

	// Si la request envía '/' respondemos con index.html si no
	// procesamos el path especificado por el navegador 
	if (strcmp(path, "/") == 0) {
		snprintf(local_path, sizeof(local_path), "%s/index.html", WEB_ROOT);
	}
	else {
		snprintf(local_path, sizeof(local_path), "%s%s", WEB_ROOT, path);
	}

	file = fopen(local_path, "rb");

	if (file == NULL) {
		send_404_response(client_fd);
	}

	else {
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Lidiamos con la petición que espera recibir 
		// el navegador 
		const char *mime_type = get_mime_type(local_path);

		const char *status_message = (status_code == 200) ? "200 OK" : "404 Not Found";

		int header_len = snprintf(header_buffer, sizeof(header_buffer), 
			"HTTP/1.0 %s\r\n"
			"Content-Type %s\r\n"
			"Connect-Length: %ld\r\n"
			"Connection: close\r\n"
			"\r\n",
			status_message, mime_type, file_size);

		// Envío de cabeceras al cliente
		write(client_fd, header_buffer, header_len);

		// Enviar contenido del archivo 
		char file_buffer[1024];
		int bytes_read;

		while((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
			if (write(client_fd, file_buffer, bytes_read) < 0) {
				perror("Error al enviar contenido del archivo");
				break;
			}
		}
	}

	fclose(file);
}
static void handle_request(int client_fd) {
	char request_buffer[BUFFER_SIZE]; 

	int bytes_received = read(client_fd, request_buffer, BUFFER_SIZE -1);
	char *method;
	if (bytes_received < 0) {
		perror("ERR: No se ha podido manejar una petición HTTP.\n");
	}

	else if (bytes_received == 0) {
		printf("El cliente terminó la conexión.\n");
	}

	else {
		request_buffer[bytes_received] = '\0';
		char *path;
		// Trucamos la primera línea de la petición
		char *request_line = strtok(request_buffer, "\r\n");
		
		if (request_line == NULL) {
			send_404_response(client_fd);
		}

		else {
			method = strtok(request_line, " ");
			path = strtok(NULL, " ");
		}

		if (method != NULL && path != NULL && strncmp(method, "GET", 3) == 0) {
			send_files(client_fd, path, 200);
		}
		else {
			send_404_response(client_fd);
		}
	}
	close(client_fd);
}

void send_404_response(int client_fd) {
	send_files(client_fd, "/404.html", 404);
}

static void accept_requests(WebServer *server) 
{
	int server_fd = server->socket_fd;

	while(server)
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
