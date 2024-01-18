# This Makefile just remove older compiled server and client programs.
# After that it just force compiles the two .c files
CC = gcc
CFLAGS = -Wall -Wextra

SERVER_SRC_DIR = server/src
CLIENT_SRC_DIR = client/src

SERVER_BIN = server/server
CLIENT_BIN = client/client

all: server client

server: $(SERVER_SRC_DIR)/server.c FORCE
	$(CC) $(CFLAGS) $< -o $(SERVER_BIN)

client: $(CLIENT_SRC_DIR)/client.c FORCE
	$(CC) $(CFLAGS) $< -o $(CLIENT_BIN)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

FORCE: