#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

int main (int argc, char *argv[])
{
    struct sockaddr_in server;
    int sockfd;
    int PORT;
    char SERVER_ADDR[50];
    // Default values for server address and port
    if (argc == 1){
        FILE *fp = fopen("config", "r");
        if (fp == NULL){
            printf("Error to open config file");
            exit(-1);
        }
        char line[50];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "ADDRESS %s", SERVER_ADDR) == 1) {
                printf("Server address: %s\n", SERVER_ADDR);
            } else if (sscanf(line, "PORT %d", &PORT) == 1) {
                printf("Port: %d\n", PORT);
            }
        }
        fclose(fp);
    // Value passed from command line
    }else if (argc == 3){
        strcpy(SERVER_ADDR, argv[1]);
        PORT = atoi(argv[2]);
        printf("Server address: %s\n", SERVER_ADDR);
        printf("Port: %d\n", PORT);
    }
    // Create socket
    if ((sockfd = socket(AF_INET,SOCK_STREAM, 0)) < 0){
        printf("Socket creation error\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) <= 0) {
        printf("Error, invalid address it isn't supported \n");
        exit(EXIT_FAILURE);
    }
    int status;
    // Connect client to server
    if ((status = connect(sockfd, (struct sockaddr*)&server, sizeof(server))) < 0) {
        printf("Connection Failed \n");
        exit(EXIT_FAILURE);
    }
    
    

    close(sockfd);
}