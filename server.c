#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#define LOG "LOG"
#define CONFIG_PATH "config/config_server"

int sockfd;
void int_handler(int signalNum);
int readConfFile(int PORT, char LOGPATH[]);
int createDir(char LOGPATH[]);

int main(int argc, char *argv[])
{
    char LOGPATH[1024];
    int PORT;
    struct sockaddr_in server,client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // Default values for port and log path
    if (argc == 1){
        // Read default values from config server file
        // --------------------------------------------------------------------------------------------------> ERRORE QUI, il server si blocca qua
        if (readConfFile(PORT,LOGPATH) < 0){
            exit(EXIT_FAILURE);
        }
    // Values passed from command line
    }else if( argc == 3){
        PORT = atoi(argv[1]);
        strcpy(LOGPATH, argv[2]);
        printf("Listening port: %d\n", PORT);
        if (createDir(LOGPATH) < 0){
            exit(EXIT_FAILURE);
        }
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
        shutdown(sockfd,SHUT_RDWR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections on a socket
    // SOMAXCONN is the maximum number of sockets in the current system
    if (listen(sockfd, SOMAXCONN) == -1) {
        printf("Error listening on server socket");
        shutdown(sockfd,SHUT_RDWR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("-------------------------------\n");
    printf("| Server started successfully |\n");
    printf("-------------------------------\n");
    // Install SIGINT signal: manage ctrl+c action by int_handler function
    signal(SIGINT, int_handler);
    // Client socket where to read data
    int clientSocket;
    while (!feof(stdin)){
        // Accept function extract the first connection on the queue of pending connections
        // pcreate a new socket with the same socket type protocol and address family as the specified socket
        if ((clientSocket = accept(sockfd,(struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            printf("Error accepting client connection...\n");

        pid_t pid = fork();

        if (pid == -1){
            printf("Error creating child process\n");
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
        }else if(pid == 0){
            // Child process
            // Shutdown and close the socket created by server to save resources
            shutdown(sockfd,SHUT_RDWR);
            close(sockfd);

            // -----------------------------------------------
            char path[1024];
            strcpy (path,LOGPATH);
            printf("%s\n",path);
            
            /*int log = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
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
            strcat(message,"\n");*/

            // -----------------------------------------------
            shutdown(clientSocket,SHUT_RDWR);
            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
    }

    shutdown(sockfd,SHUT_RDWR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}

// Handler for SIGINT signal
void int_handler(int signo){
    printf("\nSIGINT signal received, shutdown and close socket\n");
    shutdown(sockfd,SHUT_RDWR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}

int readConfFile(int PORT, char LOGPATH[])
{
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (fp == NULL){
        printf("Error to open config file");
        exit(EXIT_FAILURE);
    }
    char line[50];
    while (fgets(line, sizeof(line), fp) != NULL) {

        if (sscanf(line, "PORT %d", &PORT) == 1) 
            printf("Listening port: %d\n", PORT);
        else if (sscanf(line, "LOGPATH %s", LOGPATH) == 1) 
            printf("Log file path: %s\n", LOGPATH);

    }
    fclose(fp);
    exit(EXIT_SUCCESS);
}


int createDir(char LOGPATH[])
{   
    // If LOGPATH does not exists then create it
    struct stat st;
    if (stat(LOGPATH,&st) == -1){
        if (mkdir(LOGPATH, 644) == -1){
            printf("Error to create a dir");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}