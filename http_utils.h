#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stddef.h>

#define HTTP_OK 200
#define HTTP_NOTFOUND 404

typedef struct {
    char method[16];
    char path[4096];
    char version[16];
} HttpRequest;

void parse_http_request(char *request_in_string_format, HttpRequest *dest);
int request_has_valid_structure(HttpRequest *request);
int http_request_is_GET(HttpRequest *request);
const char *get_mime_type(const char *path);

#endif

