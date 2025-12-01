#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include "webserverlib.h"
#include "socket_utils.h"
#include "http_utils.h"

#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024

static void end_process_with_error(int error_code);
static int bind_socket_to_endpoint(int socket_fd, IPv4Endpoint *endpoint);
static int start_listening(int socket_fd);
static void handle_request(WebServer *server, int client_fd);
static void make_path_to_web_file(WebServer *server, HttpRequest *request, char *dest);
static void send_file(WebServer *server, int client_fd, char *path, int status_code);
static void send_static_404_response(int client_fd);
static void send_404_response(int client_fd);
static const char *get_mime_type(const char *path);

typedef struct 
{
	int socket_fd;
	IPv4Endpoint endpoint;
	char *WEB_ROOT_PATH;
	char *DEFAULT_WEB_PATH;
	int max_clients;
	
} WebServer;

WebServer *start_webserver(char* WEB_ROOT_PATH, char* DEFAULT_WEB_PATH, int port)
{	
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) 
		end_process_with_error(ERR_OPEN_SOCKET);
    
    IPv4Endpoint endpoint;
    endpoint = make_ipv4_endpoint("0.0.0.0", port);
    int is_binded = bind_socket_to_endpoint(socket_fd, &endpoint);
    if (!is_binded)
        end_process_with_error(ERR_BIND_SOCKET);
    
    int is_listening = start_listening(socket_fd);
    if (!is_listening)
        end_process_with_error(ERR_LISTEN);
	printf("Servidor escuchando...\n");
    
    WebServer *server = malloc(sizeof(WebServer));
    if (!server)
		end_process_with_error(ERR_MEMORY);

	server->endpoint = endpoint;
	server->socket_fd = socket_fd;
	server->max_clients = MAX_CLIENTS_IN_QUEUE;
	server->WEB_ROOT_PATH = WEB_ROOT_PATH;
	server->DEFAULT_WEB_PATH = DEFAULT_WEB_PATH;

	printf("Servidor inicializado.\n");
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

void accept_requests(WebServer *server) 
{
	int server_fd = server->socket_fd;

	while(server)
	{
		IPv4Endpoint client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

		if (client_fd >= 0)
			handle_request(server, client_fd);
		else
		{
			/*Esto se podría implementar mediante end_process_with_error 
			pero en caso de que una petición falle no queremos abortar la ejecución
			del servidor, si no que intente aceptar el resto de peticiones entrantes. */
			perror("ERR: No se ha podido aceptar una petición.");
		}
	}
}

static void handle_request(WebServer *server, int client_fd) {
	char request_buffer[BUFFER_SIZE]; 
	int bytes_received = read(client_fd, request_buffer, BUFFER_SIZE - 1);
	
	if (bytes_received < 0)
		perror("ERR: No se ha podido manejar una petición HTTP.\n");

	else if (bytes_received == 0)
		printf("El cliente terminó la conexión.\n");

	else {
		request_buffer[bytes_received] = '\0';
		HttpRequest request;
		parse_http_request(request_buffer, &request);
		
		if (http_request_has_valid_structure(request) && 
			http_request_is_GET(request)) &&
			strstr(request->path, "..")
		{
			char path_to_file[MAX_PATH_LENGTH + 5];
			make_path_to_web_file(path_to_file, server->WEB_ROOT_PATH, request->path);
			send_file(client_fd, path_to_file, HTTP_OK);
		}
		else
			send_404_response(client_fd);
	}
	close(client_fd);
}

static void make_path_to_web_file(WebServer *server, HttpRequest *request, char *dest)
{
	if (strcmp(request->path, "/"))
		strcpy(dest, server->DEFAULT_WEB_PATH);
	else
	{
		strcpy(dest, server->WEB_ROOT_PATH);
		strcat(dest, request->path);
	}		
}

static void send_file(WebServer *server, int client_fd, char *path, int status_code) {
	FILE *file = NULL; 
	long file_size = 0;
	char header_buffer[512];

	file = fopen(path, "rb");

	if (file == NULL) {
		// Si falla el puntero de lectura file no podremos
		// renderizar un HTML "bonito" y en su lugar tendremos
		// que mostrar un HTML estático a modo de fallback
		if(status_code == 404) {
			send_static_404_response(client_fd);
		}
		else {
			send_404_response(client_fd);
		}
	}

	else {
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Lidiamos con la petición que espera recibir 
		// el navegador 
		const char *mime_type = get_mime_type(path);

		const char *status_message = (status_code == 200) ? "200 OK" : "404 Not Found";

		int header_len = snprintf(header_buffer, sizeof(header_buffer), 
			"HTTP/1.0 %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %ld\r\n"
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
		fclose(file);
	}
}

static void send_static_404_response(int client_fd) {
	const char *html_body = 
        "<html><body>"
        "<h1>404 Not Found</h1>"
        "<p>El recurso solicitado no existe en este servidor.</p>"
        "</body></html>\r\n";
        
    size_t body_len = strlen(html_body); 

    char http_response[BUFFER_SIZE];
    
    int response_len = snprintf(http_response, BUFFER_SIZE,
        "HTTP/1.0 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_len, html_body);
    
    write(client_fd, http_response, response_len); 
}

static void send_404_response(int client_fd) {
	send_files(client_fd, "/404.html", HTTP_NOTFOUND);
}

void close_server(WebServer *server)
{
	close(server->socket_fd);
	free(server);
}
