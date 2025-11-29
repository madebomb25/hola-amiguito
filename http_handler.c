// HTTP handler utilities for serving static files.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "http_handler.h"

#define MAX_PATH_LENGTH 1024
#define DEFAULT_MIME_TYPE "application/octet-stream"

static const char *get_mime_type(const char *path);
static int sanitize_requested_path(const char *requested_path, char *sanitized, size_t sanitized_size);
static int build_absolute_path(char *dest, size_t dest_size, const char *web_root, const char *relative);
static void send_file_with_http_code(int client_fd, FILE *file, const char *absolute_path, int status_code);
static void send_basic_404(int client_fd);
static void url_decode(char *dst, const char *src, size_t dst_size);

void send_files(int client_fd, const char *web_root, const char *requested_path, int status_code)
{
	char relative_path[MAX_PATH_LENGTH];
	char absolute_path[MAX_PATH_LENGTH * 2];
	FILE *file = NULL;

	if (!web_root || !sanitize_requested_path(requested_path, relative_path, sizeof(relative_path))) {
		send_404_response(client_fd, web_root);
		return;
	}

	if (!build_absolute_path(absolute_path, sizeof(absolute_path), web_root, relative_path)) {
		send_404_response(client_fd, web_root);
		return;
	}

	file = fopen(absolute_path, "rb");
	if (!file) {
		send_404_response(client_fd, web_root);
		return;
	}

	send_file_with_http_code(client_fd, file, absolute_path, status_code);
	fclose(file);
}

void send_404_response(int client_fd, const char *web_root)
{
	char absolute_path[MAX_PATH_LENGTH * 2];
	FILE *file = NULL;

	if (web_root && build_absolute_path(absolute_path, sizeof(absolute_path), web_root, "404.html")) {
		file = fopen(absolute_path, "rb");
		if (file) {
			send_file_with_http_code(client_fd, file, absolute_path, 404);
			fclose(file);
			return;
		}
	}

	send_basic_404(client_fd);
}

static const char *get_mime_type(const char *path)
{
	const char *file_extension = path ? strrchr(path, '.') : NULL;
	if (!file_extension)
		return DEFAULT_MIME_TYPE;

	if (strcmp(file_extension, ".html") == 0)
		return "text/html";
	else if (strcmp(file_extension, ".css") == 0)
		return "text/css";
	else if (strcmp(file_extension, ".js") == 0)
		return "application/javascript";
	else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0)
		return "image/jpeg";
	else if (strcmp(file_extension, ".png") == 0)
		return "image/png";
	else if (strcmp(file_extension, ".gif") == 0)
		return "image/gif";
	else if (strcmp(file_extension, ".svg") == 0)
		return "image/svg+xml";
	else
		return DEFAULT_MIME_TYPE;
}

static int sanitize_requested_path(const char *requested_path, char *sanitized, size_t sanitized_size)
{
	char temp[MAX_PATH_LENGTH];
	char decoded[MAX_PATH_LENGTH];
	char *query = NULL;
	const char *source = requested_path && requested_path[0] ? requested_path : "/";
	int written = snprintf(temp, sizeof(temp), "%s", source);
	if (written < 0 || written >= (int)sizeof(temp))
		return 0;

	query = strchr(temp, '?');
	if (query)
		*query = '\0';

	url_decode(decoded, temp, sizeof(decoded));

	if (strcmp(decoded, "/") == 0) {
		strncpy(temp, "/index.html", sizeof(temp));
		temp[sizeof(temp) - 1] = '\0';
	} else {
		strncpy(temp, decoded, sizeof(temp));
		temp[sizeof(temp) - 1] = '\0';
	}

	if (strstr(temp, "..") != NULL)
		return 0;

	source = temp;
	if (*source == '/')
		source++;

	if (*source == '\0')
		source = "index.html";

	if (strlen(source) + 1 > sanitized_size)
		return 0;

	memcpy(sanitized, source, strlen(source) + 1);
	return 1;
}

static int build_absolute_path(char *dest, size_t dest_size, const char *web_root, const char *relative)
{
	if (!web_root || !relative)
		return 0;

	size_t root_len = strlen(web_root);
	const char *separator = (root_len > 0 && web_root[root_len - 1] == '/') ? "" : "/";
	int written = snprintf(dest, dest_size, "%s%s%s", web_root, separator, relative);
	return written > 0 && written < (int)dest_size;
}

static const char *status_line_for(int status_code)
{
	switch (status_code) {
	case 200:
		return "200 OK";
	case 404:
		return "404 Not Found";
	default:
		return "500 Internal Server Error";
	}
}

static void send_file_with_http_code(int client_fd, FILE *file, const char *absolute_path, int status_code)
{
	char header_buffer[512];
	char file_buffer[1024];
	long file_size;
	size_t bytes_read;
	const char *status_line = status_line_for(status_code);
	const char *mime_type = get_mime_type(absolute_path);

	if (fseek(file, 0, SEEK_END) != 0) {
		perror("ERR: No se ha podido leer el tamaño del archivo");
		return;
	}
	file_size = ftell(file);
	if (file_size < 0) {
		perror("ERR: No se ha podido obtener el tamaño del archivo");
		return;
	}
	if (fseek(file, 0, SEEK_SET) != 0) {
		perror("ERR: No se ha podido rebobinar el archivo");
		return;
	}

	int header_len = snprintf(header_buffer, sizeof(header_buffer),
		"HTTP/1.1 %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"Connection: close\r\n"
		"\r\n",
		status_line, mime_type, file_size);
	if (header_len < 0 || header_len >= (int)sizeof(header_buffer)) {
		perror("ERR: No se ha podido preparar la cabecera HTTP");
		return;
	}

	if (write(client_fd, header_buffer, header_len) < 0) {
		perror("ERR: No se ha podido enviar la cabecera HTTP");
		return;
	}

	while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
		size_t total_sent = 0;
		while (total_sent < bytes_read) {
			ssize_t sent = write(client_fd, file_buffer + total_sent, bytes_read - total_sent);
			if (sent <= 0) {
				perror("ERR: Fallo al enviar contenido del archivo");
				return;
			}
			total_sent += (size_t)sent;
		}
	}
}

static void send_basic_404(int client_fd)
{
	const char body[] =
		"<html><head><title>404</title></head>"
		"<body><h1>404 - Recurso no encontrado</h1></body></html>";
	char header_buffer[256];
	int header_len = snprintf(header_buffer, sizeof(header_buffer),
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %zu\r\n"
		"Connection: close\r\n"
		"\r\n",
		strlen(body));
	if (header_len > 0 && header_len < (int)sizeof(header_buffer))
		write(client_fd, header_buffer, header_len);
	write(client_fd, body, strlen(body));
}

static void url_decode(char *dst, const char *src, size_t dst_size)
{
	char *d = dst;
	const char *s = src;
	size_t len = 0;

	while (*s && len < dst_size - 1) {
		if (*s == '%') {
			if (s[1] && s[2]) {
				char hex[3] = {s[1], s[2], '\0'};
				*d++ = (char)strtol(hex, NULL, 16);
				s += 3;
			} else {
				*d++ = *s++;
			}
		} else {
			*d++ = *s++;
		}
		len++;
	}
	*d = '\0';
}
