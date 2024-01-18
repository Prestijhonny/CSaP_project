# This Makefile just remove older compiled server and client programs.
# After that it just force compiles the two .c files
CC = gcc
CFLAGS = -Wall -Wextra

SBIN_DIR = server/src
CBIN_DIR = client/src

all: clean server client

server: server/src/server.c FORCE
	$(CC) $< -o ../$@

client: client/src/client.c FORCE
	$(CC) $< -o ../$@

clean:
	rm -f ../server ../client

FORCE: