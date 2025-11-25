#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>

typedef struct sockaddr_in IPv4Endpoint;

static IPv4Endpoint make_ipv4_endpoint(char* ip, int port);