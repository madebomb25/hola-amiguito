#include "webserverlib.h"
#include <stdio.h>

int end_process = 0;

void ras_sigint()
{
	end_process = 1;
}

int main() {
    int port = 8081;
    
    printf("Iniciando servidor web en http://127.0.0.1:%d/\n", port);
    
    WebServer *server = start_webserver(port); 
	
	signal(SIGINT, ras_sigint);
	
    if (server != NULL) {
        accept_requests(server); 
        close_server(server);
    }
}
