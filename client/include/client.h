#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#define TRUE 1
#define MAX_HOSTNAME 1024

pid_t PPID;
int sockfd;

void int_handler(int signalNum);
void checkServerConnection(int server_socket);


// Handler for SIGINT signal
void int_handler(int signo){

    if (PPID == getpid()){
        printf("\nSIGINT signal received, shutdown and close socket for all processes\n");
    }
    shutdown(sockfd,SHUT_RDWR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}

void checkServerConnection(int server_socket) {
    ssize_t bytesRead = recv(server_socket, NULL, 0, MSG_PEEK);
    if (bytesRead == 0) {
        printf("Server has disconnected\n");
        kill(PPID,SIGINT);
        close(sockfd);
        exit(EXIT_SUCCESS);
    }else if (bytesRead == -1){
        printf("Error receiving data\n");
        kill(PPID,SIGINT);
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}