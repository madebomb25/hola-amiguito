#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "webserverlib.h"
#include "socket_utils.h"
#include "http_utils.h"
#include "cache.h"

#define MAX_CLIENTS_IN_QUEUE 5
#define ERR_OPEN_SOCKET -1
#define ERR_BIND_SOCKET -2
#define ERR_LISTEN -3
#define ERR_MEMORY -4

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024

static CacheEntry cache[MAX_CACHE_ENTRIES];
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct 
{
	int socket_fd;
	IPv4Endpoint endpoint;
	char *WEB_ROOT_PATH;
	int max_clients;
	
} WebServer;

typedef struct {
	WebServer *server;
	int client_fd;
} ThreadRequestData;

WebServer *start_webserver(char* WEB_ROOT_PATH, int port)
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

	printf("Servidor inicializado.\n");
	
	init_cache(cache);
	
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
		socklen_t client_addr_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd >= 0)
			let_thread_handle_request(server, client_fd);
			
		else
		{
			/*En caso de que una petición falle no queremos abortar la ejecución
			del servidor, si no que intente aceptar el resto de peticiones entrantes. */
			perror("ERR: No se ha podido aceptar una petición.");
		}
	}
}

static void let_thread_handle_request(WebServer *server, int client_fd)
{	
	ThreadRequestData *thread_data = malloc(sizeof(ThreadRequestData));
	thread_data->server = server;
	thread_data->client_fd = client_fd;
	
	pthread_t tid;
	
	/* Creamos un nuevo hilo por cada nueva conexión para repartir
	 * la carga en varios núcleos del procesador.*/
	 
	if (pthread_create(&tid, NULL, thread_function, thread_data) != 0) {
		perror("ERR: No se ha podido asignar un hilo a una petición.");
		close(client_fd);
		free(thread_data);
	}
	else
		pthread_detach(tid);
}

void* thread_function(void *arg){
	ThreadRequestData *data = (ThreadRequestData*) arg;

	handle_request(data->server, data->client_fd);
	free(arg);

	pthread_exit(NULL);
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
			http_request_is_GET(&request) &&
			!has_illegal_path(&request))
		{	
			send_files(server, client_fd, request->path, HTTP_OK);
		}
		else
			send_404_response(server, client_fd);
	}
	close(client_fd);
}

static int has_illegal_path(HttpRequest *request)
{
	// Se pueden añadir mas expresiones que sepamos que dan accesos
	// no autorizados...
	return strstr(request->path, "..");
}

static void send_404_response(WebServer *server, int client_fd) {
	send_files(server, client_fd, "/404.html", HTTP_NOTFOUND);
}

static void send_files(WebServer* server, int client_fd, char *request_path, int status_code) {
	char local_path[MAX_PATH_LENGTH + 5]; 
	char cache_key[MAX_PATH_LENGTH];
	char header_buffer[512];
	
	char *web_root = server->WEB_ROOT_PATH;
	
	if (strcmp(request_path, "/") == 0) {
		snprintf(local_path, sizeof(local_path), "%s/index.html", web_root);
		snprintf(cache_key, sizeof(cache_key), "/index.html");
	}
	else {
		snprintf(local_path, sizeof(local_path), "%s%s", web_root, request_path);
		snprintf(cache_key, sizeof(cache_key), "%s", request_path);
	}

	const char *mime_type = get_mime_type(local_path);
	const char *status_message = (status_code == HTTP_OK) ? "200 OK" : "404 Not Found";

	size_t file_size;
	char *response_data = NULL;
	int from_cache = 0;   // 0 = MISS, 1 = HIT

	pthread_mutex_lock(&cache_mutex);
	CacheEntry *entry = find_in_cache(cache, cache_key);

	if (entry && is_cache_valid(entry, local_path)) {
    	entry->last_access = time(NULL);
        file_size = entry->size;
        response_data = entry->data;
		from_cache = 1;
		
        pthread_mutex_unlock(&cache_mutex);
	}
	else {
		pthread_mutex_unlock(&cache_mutex);

		time_t mtime;
		char *data = load_file(local_path, &file_size, &mtime);

		if (!data) {
			// Si falla el puntero de lectura file no podremos
			// renderizar un HTML "bonito" y en su lugar tendremos
			// que mostrar un HTML estático a modo de fallback
			if (status_code == HTTP_NOTFOUND) {
				send_static_404_response(server, client_fd);
			} else {
				send_404_response(server, client_fd);
			}
			return;
		}
		pthread_mutex_lock(&cache_mutex);
		store_in_cache(cache, cache_key, local_path, data, file_size, mtime);
		pthread_mutex_unlock(&cache_mutex);

		response_data = data;
	}

	int header_len = snprintf(header_buffer, sizeof(header_buffer), 
		"HTTP/1.0 %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"Connection: close\r\n"
		"\r\n",
		status_message, mime_type, file_size);
	
	printf("[CACHE] %s: %s (size=%zu)\n",
       from_cache ? "HIT" : "MISS", cache_key, file_size);
	
	// Envío de cabeceras al cliente
	write(client_fd, header_buffer, header_len);

	size_t offset = 0;
	while (offset < file_size){
		size_t chunk = (file_size - offset >= 1024) ? 1024 : file_size - offset;
		if (write(client_fd, response_data + offset, chunk) < 0) {
			perror("Error al enviar contenido del archivo");
		}
	offset += chunk;
	}
}

static void send_static_404_response(WebServer *server, int client_fd) {
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

void close_server(WebServer *server)
{
	close(server->socket_fd);
	free(server);
}
