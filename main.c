#include "webserverlib.h"
#include <string.h>

#define WEB_ROOT "/var/www/html"

void client_thread(void *arg){
    int client_fd = *(int*) arg;
    free(arg);

    char buffer[1024];
    int n = read(client_fd, buffer, sizeof(buffer));

    if(n <= 0){
        // Error at connecting or user has closed the connection.
        perror("Connection has been closed by an ocurred error.");
        close(client_fd);
        return NULL;
    }

    buffer[n] = '\0';

    // Definition of usable files at web presentation
    char filepath[256] = WEB_ROOT;
    char requested_file[128];

    // Directory parsing
    char *path_start = strchr(buffer, ' '); // 
    if(!path_start){
        close(client_fd);
        return NULL;
    }
    ++path_start;

    char *path_end = strchr(path_start, ' '); //
    if(!path_end){
        close(client_fd);
        return NULL;
    }
    int path_len = path_end - path_start;
    if(path_len >= sizeof(requested_file)) path_len = sizeof(requested_file)-1;
    strncpy(requested_file, path_start, path_len);
    requested_file[path_len] = '\0';
    strcat(filepath, requested_file);

    // File opening for presentation
    FILE *f = fopen(filepath, "r");
    
    if(!f){
        char *msg = "HTTP/1.1 404 Not Found.\n";
        write(client_fd, msg, len(msg));
        close(client_fd);
        return NULL;
    }

    // Connection done succesfully    
    // HEADER 
    char *msg = "HTTP/1.1 200 OK.";
    write(client_fd, msg, len(msg));

    // DYNAMIC CONTENT-TYPE
    if(strstr(filepath, ".html")){
        msg [] = "Content-Type: text/html";
        write(client_fd, msg, len(msg));
    } else if(strstr(filepath, ".css")){
        msg [] = "Content-Type: text/html";
        write(client_fd, msg, len(msg));
    } // AGREGAR MÁS EN CASO DE USAR .JS O OTROS
    
    // CONTENT
    char filebuf[1024];
    int bytes;
    // Write document content chunk by chunk
    while((bytes = fread(filebuf, 1, sizeof(filebuf), f)) > 0){
        write(client_fd, filebuf, bytes);
    }

    close(client_fd);
    return NULL;
}

void* web_server_thread(void *arg){

    int server_fd = start_webserver(SERVER_PORT);
    if(server_fd < 0){
        // Error handling at server starting
        if(server_fd == ERR_BIND_SOCKET){ 
            perror("Unable to bind socket.");
            return NULL;
        }
        if(server_fd == ERR_OPEN_SOCKET){ 
            perror("Unable to open socket.");
            return NULL;
        }
        if(server_fd == ERR_LISTEN){
            perror("Unable to listen.");
            return NULL;
        }

    } else {
        while(1){
            int client_fd = accept(server_fd, NULL, NULL);
            if(client_fd < 0){ 
                perror("Unable to accept.");    
                continue;
            }
            // In other hand, if client_fd >= 0, the connection has been created succesfully.
        
            pthread_t tid;
            int *pfd = malloc(sizeof(int)); // Memory has been reserved for pfd
            *pfd = client_fd;
            pthread_create(&tid, NULL, client_thread, pfd);
            pthread_detach(tid); 
        }

        // To a presistent server, there is no way close server chanel, so it doesn't arrive here.
        close(server_fd);
        return NULL;
    }
}


int main(){
    
}