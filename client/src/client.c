#include "../include/client.h"

extern int sockfd;
extern pid_t PPID;

int main (int argc, char *argv[])
{
    struct sockaddr_in server;
    int PORT;
    char SERVER_ADDR[INET_ADDRSTRLEN];
    char HOSTNAME[MAX_HOSTNAME];
    struct hostent *host;
    // Get parent process pid for using in the SIGINT handler
    PPID = getpid();
    // This means that the client will read default conf as server address and port
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
    // The case where the conf are passed by the command-line
    }else if (argc == 3){
        strcpy(HOSTNAME, argv[1]);
        PORT = atoi(argv[2]);
        // Resolve hostname to IP address
        if ((host = gethostbyname(HOSTNAME)) == NULL){
            printf("Error to resolve hostname\n");
            exit(EXIT_FAILURE);
        }
        // I'm copying the IP address from the first element of list handled by "struct hostent *host" variable
        server.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
        // inet_ntop() converts the binary IP address in server.sin_addr to a human-readable string format
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
    // SOL_SOCKET indicates the level argument specifies the protocol level at which the option resides
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
    // Register a signal SIGINT (CTRL+C when pressed on cmd) and SIGUSR1 (CTRL+D when pressed on cmd) 
    signal(SIGINT, handler);
    signal(SIGUSR1, handler);
    pid_t pid = fork();
    // I created two process in client for these reasons:
    // 1) Child process manages the messages sent to server: it uses fgets and send to send data to server 
    // 2) Parent process is listening on socket in MSG_PEEK mode by recv. This means it only waits for server disconnection, that is, when the recv returns 0
    char data[MAX_DATA];
    if (pid == 0){
        // Child process
        // Flush the string 
        memset(data, 0, sizeof(data));
        printf("Send some data (press CTRL+D or type exit to quit): ");
        // While loop until read EOF on stdin
        // Get data from stdin
        while (fgets(data, sizeof(data), stdin) != NULL) {
            printf("Send some data (press CTRL+D or type exit to quit): ");

            // Send data to server 
            if (send(sockfd,data,strlen(data),0) == -1){
                printf("Error to send data\n");
                kill(PPID, SIGINT);
                shutdown(sockfd,SHUT_RDWR); 
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            memset(data, 0, sizeof(data));
        }
        printf("\n\nCTRL+d pressed, read OEF from stdin\n");
        kill(0, SIGUSR1);
    }else if (pid > 0){
        // Parent process
        // Wait until server is disconnected
        ssize_t bytesRead = recv(sockfd, NULL, 0, MSG_PEEK);
        
        if (bytesRead == 0) 
            printf("\n\nThe server has disconnected\n");
        else if (bytesRead == -1)
            printf("\nError in recv\n");

        // Send SIGINT signal to child process and waits the termination of child to terminate also the parent process
        // I could just send kill to every related processes (kill(0,SIGINT)) but i want to avoid to print the message in the handler about SIGINT
        kill(pid, SIGINT);
        wait(NULL);

        // Shutdown and close the socket
        shutdown(sockfd, SHUT_RDWR); 
        close(sockfd); 
        printf("\nShutdown and close socket...\n");
        // Exit with appropriate status
        if (bytesRead == 0) 
            exit(EXIT_SUCCESS);
        else if (bytesRead == -1)
            exit(EXIT_FAILURE);

    }else{
        printf("Error creating child process\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_SUCCESS);
    }
}
