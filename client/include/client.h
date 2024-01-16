#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#define TRUE 1
#define MAX_HOSTNAME 1024
#define MAX_DATA 2048

pid_t PPID;
int sockfd;

void int_handler(int signalNum);


// Handler for SIGINT signal
void int_handler(int signo){

    if (PPID == getpid())
        printf("\nSIGINT signal received, shutdown and close socket\n");
    shutdown(sockfd,SHUT_RDWR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}