#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <http_utils.h>

#define HTTP_OK 200
#define HTTP_NOTFOUND 404

typedef struct {
    char method[16];
    char path[4096];
    char version[16];
} HttpRequest;

void parse_http_request(char *request_in_string_format,
									 HttpRequest *dest)
{
	char *method_token = strtok(request_in_string_format, " \r\n");
    if (method_token)
        snprintf(dest->method, sizeof(dest->method), "%s", method_token);
    
    char *path_token = strtok(NULL, " \r\n");
    if (path_token)
        snprintf(dest->path, sizeof(dest->path), "%s", path_token);
    
    char *version_token= strtok(NULL, " \r\n");
    if (version_token)
        snprintf(dest->version, sizeof(dest->version), "%s", version_token);
}

int request_has_valid_structure(HttpRequest *request)
{
	// Como un array nunca será nulo, hay que revisar que al menos exista
	// algo en la primera posición.
	return request->method[0] && request->path[0] && request->version[0];
}

int http_request_is_GET(HttpRequest *request)
{
	return strncmp(request->method, "GET", 3) == 0;
}

char *get_mime_type(const char *path) {
	const char *file_extension = strrchr(path, '.');
	const char *DEFAULT_RESPONSE = "application/octet-stream";

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
