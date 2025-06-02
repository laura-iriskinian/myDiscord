CC = gcc
CFLAGS = -Wall -pthread -I/usr/include/postgresql
LDFLAGS = -pthread -lpq

server: server_main.c server.c
	$(CC) $(CFLAGS) -o server server_main.c server.c $(LDFLAGS)

clean:
	rm -f server