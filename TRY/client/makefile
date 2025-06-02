CC = gcc
CFLAGS = -Wall -pthread `pkg-config --cflags gtk+-3.0`
LDFLAGS = -pthread `pkg-config --libs gtk+-3.0`

client: main.c client.c
	$(CC) $(CFLAGS) -o client main.c client.c $(LDFLAGS)

clean:
	rm -f client