# This Makefile just remove older compiled server and client programs.
# After that it just force compiles the two .c files
CC = gcc
CFLAGS = -Wall -Wextra

SERVER_DIR = server/src
CLIENT_DIR = client/src

all: server client

server: $(SERVER_DIR)/server.c FORCE
	$(CC) $(CFLAGS) $< -o $(SERVER_DIR)/$@

client: $(CLIENT_DIR)/client.c FORCE
	$(CC) $(CFLAGS) $< -o $(CLIENT_DIR)/$@

clean:
	rm -f $(SERVER_DIR)/server $(CLIENT_DIR)/clientp

FORCE: