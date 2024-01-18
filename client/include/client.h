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

// I need this global variables here for handler 
pid_t PPID;
int sockfd;

void handler(int signalNum);


// Handler for SIGINT signal
void handler(int signo){
    // I use this "if" to make this printf to be used only by the parent process 
    if (PPID == getpid()){
        if(signo == SIGINT)
            printf("\nSIGINT signal received, shutdown and close socket\n");
        // Wait until the child process dies
        wait(NULL);
    }
    shutdown(sockfd,SHUT_RDWR); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}