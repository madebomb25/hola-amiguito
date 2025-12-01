#include "webserverlib.h"
#include <stdio.h>

int main() {
    int port = 8081;
    
    printf("Iniciando servidor web en http://127.0.0.1:%d/\n", port);
    
    
    WebServer *server = start_webserver(port); 

    if (server != NULL) {
        accept_requests(server); 
        end_server(server);
    } 
    
    return 0;
}