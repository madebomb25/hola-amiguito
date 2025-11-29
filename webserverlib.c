#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http_handler.h"
#include "webserverlib.h"

#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

#define BUFFER_SIZE 4096

extern volatile sig_atomic_t end_process;

static void end_process_with_error(int error_code);
static int bind_socket_to_endpoint(int socket_fd, IPv4Endpoint *endpoint);
static int start_listening(int socket_fd);
static void handle_request(WebServer *server, int client_fd);

WebServer *start_webserver(const char *web_root_path, int port)
{
	int socket_fd;
	int opt_value = 1;
	IPv4Endpoint endpoint;
	WebServer *server = NULL;

	if (!web_root_path)
		end_process_with_error(ERR_MEMORY);

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0)
		end_process_with_error(ERR_OPEN_SOCKET);

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

	endpoint = make_ipv4_endpoint("0.0.0.0", port);
	if (!bind_socket_to_endpoint(socket_fd, &endpoint))
		end_process_with_error(ERR_BIND_SOCKET);

	if (!start_listening(socket_fd))
		end_process_with_error(ERR_LISTEN);

	server = malloc(sizeof(WebServer));
	if (!server)
		end_process_with_error(ERR_MEMORY);

	server->endpoint = endpoint;
	server->socket_fd = socket_fd;
	server->max_clients = MAX_CLIENTS_IN_QUEUE;
	server->WEB_ROOT_PATH = strdup(web_root_path);
	if (!server->WEB_ROOT_PATH)
		end_process_with_error(ERR_MEMORY);

	return server;
}

void accept_requests(WebServer *server)
{
	fd_set read_fds;
	struct timeval timeout;
	int server_fd;

	if (!server)
		return;

	server_fd = server->socket_fd;
	while (!end_process) {
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int ready = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
		if (ready < 0) {
			if (errno == EINTR)
				continue;
			perror("ERR: No se ha podido esperar conexiones entrantes");
			break;
		}

		if (ready == 0)
			continue;

		if (FD_ISSET(server_fd, &read_fds)) {
			IPv4Endpoint client_addr;
			socklen_t client_len = sizeof(client_addr);
			int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
			if (client_fd < 0) {
				if (errno == EINTR)
					continue;
				perror("ERR: No se ha podido aceptar una petición.");
				continue;
			}

			handle_request(server, client_fd);
		}
	}
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

static void handle_request(WebServer *server, int client_fd)
{
	char request_buffer[BUFFER_SIZE];
	ssize_t bytes_received;
	char *method = NULL;
	char *requested_path = NULL;
	char *request_line = NULL;

	bytes_received = read(client_fd, request_buffer, sizeof(request_buffer) - 1);
	if (bytes_received <= 0) {
		if (bytes_received < 0)
			perror("ERR: No se ha podido manejar una petición HTTP");
		close(client_fd);
		return;
	}

	request_buffer[bytes_received] = '\0';
	request_line = strtok(request_buffer, "\r\n");
	if (!request_line) {
		send_404_response(client_fd, server->WEB_ROOT_PATH);
		close(client_fd);
		return;
	}

	method = strtok(request_line, " ");
	requested_path = strtok(NULL, " ");

	if (method && requested_path && strcmp(method, "GET") == 0)
		send_files(client_fd, server->WEB_ROOT_PATH, requested_path, 200);
	else
		send_404_response(client_fd, server->WEB_ROOT_PATH);

	close(client_fd);
}

void close_server(WebServer *server)
{
	if (!server)
		return;

	if (server->socket_fd >= 0)
	{
		close(server->socket_fd);
		server->socket_fd = -1;
	}
	free(server->WEB_ROOT_PATH);
	free(server);
}
