#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#define HTTP_OK 200
#define HTTP_NOTFOUND 404
#define MAX_METHOD_LEN 16
#define MAX_VERSION_LEN 16
#define MAX_PATH_LEN 4096

typedef struct {
    char method[MAX_METHOD_LEN];
    char path[MAX_PATH_LEN];
    char version[MAX_VERSION_LEN];
} HttpRequest;

void parse_http_request(char *request_in_string_format, HttpRequest *dest);
int http_request_has_valid_structure(HttpRequest *request);
int http_request_is_GET(HttpRequest *request);
const char *get_mime_type(const char *path);

#endif

