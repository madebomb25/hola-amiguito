static const char *get_mime_type(const char *path) {
	const char *file_extension = strrchr(path, '.');
	const char DEFAULT_RESPONSE = "application/octet-stream";

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

void send_404_response(int client_fd) {
	send_files(client_fd, "/404.html", 404);
}

void send_file_with_http_code(int client_fd, char* path) {
	
	char local_path[MAX_PATH_LENGTH + 5]; 
	FILE *file = NULL; 
	long file_size = 0;
	char header_buffer[512]; 

	// Si la request envía '/' respondemos con index.html si no
	// procesamos el path especificado por el navegador

	file = fopen(local_path, "rb");

	if (file == NULL) {
		send_404_response(client_fd);
	}

	else {
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Lidiamos con la petición que espera recibir 
		// el navegador 
		const char *mime_type = get_mime_type(local_path);

		const char *status_message = (status_code == 200) ? "200 OK" : "404 Not Found";

		int header_len = snprintf(header_buffer, sizeof(header_buffer), 
			"HTTP/1.0 %s\r\n"
			"Content-Type %s\r\n"
			"Connect-Length: %ld\r\n"
			"Connection: close\r\n"
			"\r\n",
			status_message, mime_type, file_size);

		// Envío de cabeceras al cliente
		write(client_fd, header_buffer, header_len);

		// Enviar contenido del archivo 
		char file_buffer[1024];
		int bytes_read;

		while((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
			if (write(client_fd, file_buffer, bytes_read) < 0) {
				perror("Error al enviar contenido del archivo");
				break;
			}
		}
	}

	fclose(file);
}
