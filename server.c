#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For exit
#include <string.h>     // For strlen, memset
#include <unistd.h>     // For read, write, close
#include <sys/socket.h> // For socket definitions and functions
#include <netinet/in.h> // For sockaddr_in structure

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd;       // File descriptor for the main server socket
    int new_socket;      // File descriptor for the new connecting client
    struct sockaddr_in address; 
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0}; // Buffer to store the incoming request data

    // --- 1. CREATE SOCKET ---
    // AF_INET = IPv4 protocol family
    // SOCK_STREAM = TCP (connection-oriented)
    // 0 = Default protocol (IP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // --- 2. BIND TO PORT ---
    // Prepare the address structure for binding
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // Listen on any network interface (0.0.0.0)
    address.sin_port = htons(PORT);         // Convert port number to network byte order

    // Bind the socket to the specified IP and Port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // --- 3. LISTEN ---
    // Start listening for incoming connections. '3' is the backlog queue size.
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running. Waiting for connections on port %d...\n", PORT);
    // 

    // --- 4. MAIN LOOP ---
    // Loop indefinitely to accept and handle multiple client requests
    while (1) {
        printf("\nWaiting for a new connection...\n");

        // --- 5. ACCEPT ---
        // Block until a client attempts a connection. 
        // A new socket file descriptor (new_socket) is created for communication.
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue; // Skip the rest of the loop and wait for the next connection
        }

        // --- 6. READ REQUEST ---
        // Clear buffer before reading
        memset(buffer, 0, BUFFER_SIZE);
        // Read the data sent by the client (HTTP request)
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Message received from client:\n%s\n", buffer);

        // --- 7. BUILD RESPONSE ---
        // A simple HTTP/1.1 response with a plain text body.
        // \r\n\r\n separates the HTTP headers from the body.
        char *http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n" 
            "Content-Length: 20\r\n"
            "\r\n"
            "Hola Amiguito Server"; // Body content is 20 characters long

        // --- 8. WRITE RESPONSE ---
        // Send the HTTP response back to the client
        write(new_socket, http_response, strlen(http_response));
        printf("Response sent to client.\n");

        // --- 9. CLOSE ---
        // Close the communication socket for this specific client connection
        close(new_socket);
    }

    // This line is technically unreachable due to the infinite while(1) loop, 
    // but included for function completeness.
    return 0; 
}
// This closing brace for main() was the cause of the original error.