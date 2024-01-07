CC = gcc
CFLAGS = -Wall -Wextra

SBIN_DIR = server/src
CBIN_DIR = client/src

all: server client

server: server/src/server.c
	$(CC) $^ -o $(SBIN_DIR)/$@

client: client/src/client.c
	$(CC) $^ -o $(CBIN_DIR)/$@

clean:
	rm -f server client