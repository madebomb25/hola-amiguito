TARGET = webserver
SRC = main.c http_utils.c socket_utils.c cache.c webserverlib.c
CFLAGS = -Wall -Wextra -pthread

all:
	gcc $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
