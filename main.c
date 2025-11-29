#include <signal.h>

#include "webserverlib.h"

#define PORT 8080

volatile sig_atomic_t end_process = 0;

static void ras_sigint(int sig)
{
	(void)sig;
	end_process = 1;
}

int main(void)
{
	WebServer *server = start_webserver("web", PORT);

	signal(SIGINT, ras_sigint);
	accept_requests(server);
	close_server(server);
	return 0;
}
