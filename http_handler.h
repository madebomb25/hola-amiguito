#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

void send_files(int client_fd, const char *web_root, const char *requested_path, int status_code);
void send_404_response(int client_fd, const char *web_root);

#endif
