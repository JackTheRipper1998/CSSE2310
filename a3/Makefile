CC = gcc
CFLAGS = -pedantic -Wall -std=gnu99 -g
TARGETS = client clientbot server

.PHONY: all clean
.DEFAULT_GOAL := all

all: $(TARGETS)

server: server.c function.o
	$(CC) $(CFLAGS) -o server server.c function.o

clientbot: clientbot.c function.o
	$(CC) $(CFLAGS) -o clientbot clientbot.c function.o

client: client.c function.o
	$(CC) $(CFLAGS) -o client client.c function.o

function.o: function.c
	$(CC) $(CFLAGS) -c function.c

clean:
	rm -f $(TARGETS) *.o