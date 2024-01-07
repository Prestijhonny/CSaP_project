#include "../include/client.h"

int sockfd;

int main (int argc, char *argv[])
{
    struct sockaddr_in server;
    int PORT;
    char SERVER_ADDR[INET_ADDRSTRLEN];
    char HOSTNAME[MAX_HOSTNAME];
    struct hostent *host;
    // Default values for server address and port
    if (argc == 1){
        FILE *fp = fopen("../../config/config_client", "r");
        if (fp == NULL){
            printf("Error to open config file");
            exit(-1);
        }
        char line[50];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "ADDRESS %s", HOSTNAME) == 1) {
                printf("Server hostname: %s\n", HOSTNAME);
            } else if (sscanf(line, "PORT %d", &PORT) == 1) {
                printf("Port: %d\n", PORT);
            }
        }
        fclose(fp);
    // Values passed from command line
    }else if (argc == 3){
        strcpy(HOSTNAME, argv[1]);
        PORT = atoi(argv[2]);
        // Resolve hostname to IP address
        if ((host = gethostbyname(HOSTNAME)) == NULL){
            printf("Error to resolve hostname\n");
            exit(EXIT_FAILURE);
        }
        server.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
        // inet_ntop() converts IP address to textual format  
        if (inet_ntop(AF_INET, &server.sin_addr, SERVER_ADDR, sizeof(SERVER_ADDR)) == NULL) {
            printf("inet_ntop() failed");
            exit(EXIT_FAILURE);
        }
        // If hostname and server address are equal it means that the values passed on cmd are an IP and PORT instead of HOSTNAME and PORT
        if (strcmp(HOSTNAME, SERVER_ADDR) != 0)
            printf("Server hostname: %s\n", HOSTNAME);
        
        printf("IP address: %s\n", SERVER_ADDR);
        printf("Port: %d\n", PORT);
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
    server.sin_port = htons(PORT);

    int opt = 1;
    // Setup some options for current socket, SO_REUSEADDR is an option that permit to reuse immediately a local address, although it is still used by another socket
    if ((setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR,(char *)&opt, sizeof(opt))) == -1){
        printf("Error to setup options to socket\n");
        shutdown(sockfd,SHUT_RDWR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int status;
    // Connect client to server
    if ((status = connect(sockfd, (struct sockaddr*)&server, sizeof(server))) < 0) {
        printf("Connection Failed\n");
        shutdown(sockfd,SHUT_RDWR); 
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    // Get current time
    time_t currentTime;
    time(&currentTime);
    // Convert time to string representation
    char *timeString = ctime(&currentTime);
    printf("------------------------------------------------------------\n");
    printf("Successfully connected to server at %s",timeString);
    printf("------------------------------------------------------------\n");
    // Register a signal SIGINT (CTRL+c when pressed on cmd)
    signal(SIGINT, int_handler);

    // While loop until read EOF on stdin
    char data[2048];
    memset(data, 0, sizeof(data));
    // Get data from stdin
    printf("Send some data (press CTRL+D to quit): ");
    while (fgets(data, sizeof(data), stdin) != NULL) {
        printf("Send some data (press CTRL+D to quit): ");
        
        // Send data to server 
        if (send(sockfd,data,strlen(data),0) == -1){
            printf("Error to send data\n");
            shutdown(sockfd,SHUT_RDWR); 
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        memset(data, 0, sizeof(data));
    }
    printf("\nShutdown and close socket...\n");
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