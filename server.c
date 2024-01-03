#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <signal.h>

int sockfd;
void int_handler(int signalNum);

int main(int argc, char *argv[])
{
    char LOGPATH[1024];
    int PORT;
    struct sockaddr_in server,client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // Default values for port and log path
    if (argc == 1){
        // Read default values from config server file
        FILE *fp = fopen("config/config_server", "r");
        if (fp == NULL){
            printf("Error to open config file");
            exit(-1);
        }
        char line[50];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "PORT %d", &PORT) == 1) {
                printf("Listening port: %d\n", PORT);
            } else if (sscanf(line, "LOGPATH %s", LOGPATH) == 1) {
                printf("Log file path: %s\n", LOGPATH);
            }
        }
        fclose(fp);
    // Values passed from command line
    }else if( argc == 3){
        PORT = atoi(argv[1]);
        strcpy(LOGPATH, argv[2]);
        printf("Listening port: %d\n", PORT);
        printf("Log file path: %s\n", LOGPATH);
    }else if (argc == 2){
        printf("Error too few arguments entered\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Error too many arguments entered\n");
        exit(EXIT_FAILURE);
    }
    
    // Create socket
    if ((sockfd = socket(AF_INET,SOCK_STREAM, 0)) < 0){
        printf("Socket creation error\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        printf("Error binding server socket\n");
        shutdown(sockfd,SHUT_WR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections on a socket
    // Chiedere al prof del numero di client
    if (listen(sockfd, 1000) == -1) {
        printf("Error listening on server socket");
        shutdown(sockfd,SHUT_WR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, int_handler);
    int clientSocket;
    while (!feof(stdin)){
        if ((clientSocket = accept(sockfd,(struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            printf("Error accepting client connection...\n");

        pid_t pid = fork();
        if (pid == -1){
            printf("Error creating child process\n");
            shutdown(clientSocket, SHUT_WR);
            close(clientSocket);
        }else if(pid == 0){
            // Child process
            char path[1024];
            strcpy (path,LOGPATH);
            strcat(path, "/log1");
            
            int log = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (log == -1){
                printf ("Error opening file\n");
                exit(EXIT_FAILURE);
            }
            // Get current time
            time_t currentTime;
            time(&currentTime);
            // Convert time to string representation
            char *timeString = ctime(&currentTime);
            write(log,timeString,strlen(timeString));
            char clientAddr[INET_ADDRSTRLEN], logAddress[1024] = "Client address: ";
            if (inet_ntop(AF_INET, &(client_addr.sin_addr), clientAddr, sizeof(clientAddr)) == NULL) {
                printf("Error converting client address to string");
                exit(EXIT_FAILURE);
            }
            strcat(logAddress, clientAddr);
            write(log,logAddress,strlen(logAddress));
            char message[1024];
            // Sistemare qui
            if (recv(clientSocket,message,sizeof(message),0) == -1){
                printf("Error to receive data from socket\n");
                exit(EXIT_FAILURE);
            }
            write(log,message,strlen(message));
            strcat(message,"\n");
            exit(EXIT_SUCCESS);
        }
    }

    shutdown(sockfd,SHUT_WR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}

// Handler for SIGINT signal
void int_handler(int signo){
    printf("\nSIGINT signal received, shutdown and close socket\n");
    shutdown(sockfd,SHUT_WR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}