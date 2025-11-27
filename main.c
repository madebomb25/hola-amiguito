#include <stdio.h>
#include <stdlib.h>
#include "webserverlib.h"

#define PORT 8080

int end_process = 0;

void ras_sigint()
{
	end_process = 1;
}

int main()
{
	WebServer *server = start_webserver("www", PORT);
	
	signal(SIGINT, ras_sigint);
	
	while(!end_process);
	
	close_server(server);
}
