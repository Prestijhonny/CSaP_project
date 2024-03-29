#include "../include/server.h"

extern sem_t sem;
extern int sockfd, clientSocket;
extern pid_t PPID;
extern char LOGPATH[MAX_PATH];

int main(int argc, char *argv[])
{
    int PORT;
    struct sockaddr_in server, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Get parent process pid
    PPID = getpid();
    // Default values for port and log path
    if (argc == 1)
    {
        // Read default values from config server file
        if (readConfFile(&PORT) < 0)
            exit(EXIT_FAILURE);
        // Create the default dir if it doesn't exists
        if (createDir() < 0)
            exit(EXIT_FAILURE);
    }
    else if (argc == 3) // Values passed from command line
    {
        PORT = atoi(argv[1]);
        strcpy(LOGPATH, "../../");
        strcat(LOGPATH, argv[2]);
        if (createDir() < 0)
            exit(EXIT_FAILURE);
        printf("Listening port: %d\n", PORT);
        printf("Log file path: %s\n", LOGPATH);
    }
    else if (argc == 2)
    {
        printf("Error too few arguments entered\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Error too many arguments entered\n");
        exit(EXIT_FAILURE);
    }
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
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
    
    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("Error binding server socket\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections on a socket
    // SOMAXCONN is the maximum number of sockets in the current system
    if (listen(sockfd, SOMAXCONN) == -1)
    {
        printf("Error listening on server socket");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Install SIGINT signal: manage ctrl+c action by handler function
    signal(SIGINT, handler);

    printf("-------------------------------\n");
    printf("| Server started successfully |\n");
    printf("-------------------------------\n");

    // Get current time
    time_t currentTime;
    time(&currentTime);
    // Convert time to string representation
    char *timeString = ctime(&currentTime);
    printf("\nDate and time: %s\n",timeString);
    

    // Init unamed semaphore
    if (sem_init(&sem,0,1)  == -1){
        printf("Error initializing semaphore\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (TRUE)
    {
        // Accept function extract the first connection on the queue of pending connections
        // create a new socket with the same socket type protocol and address family as the specified socket
        if ((clientSocket = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            printf("Error accepting client connection...\n\n");
        else{
            // If the connection has accepted correctly from server
            
            char clientAddr[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(client_addr.sin_addr), clientAddr, sizeof(clientAddr)) == NULL){
                printf("Error converting client address to string");
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
                exit(EXIT_FAILURE);
            }
            
            pid_t pid = fork();

            if (pid == -1)
                printf("Error creating child process to manage client\n");
            else if (pid == 0)
            {
                // Child process
                // Close the socket created by server to save resources
                close(sockfd);
                int intPortOfClient = ntohs(client_addr.sin_port);

                printf("A client has connected, accepted connection from %s:%d\n\n", clientAddr, intPortOfClient);

                char acceptedClient[128];
                memset(acceptedClient, 0, sizeof(acceptedClient));
                snprintf(acceptedClient, sizeof(acceptedClient), "A client has connected, accepted connection from %s:%d\n\n", clientAddr, intPortOfClient);
                sem_wait(&sem);
                // Write on log file that the client has connected
                FILE *fp = getFileDescriptor(strlen(acceptedClient));
                write(fileno(fp),acceptedClient,strlen(acceptedClient));
                fclose(fp);
                sem_post(&sem);

                // Function to handle the client connection
                int value = handleClientConn(clientSocket, clientAddr, intPortOfClient);

                // Release, close, and destroy the semaphore
                sem_post(&sem);
                sem_close(&sem);
                sem_destroy(&sem);

                // Shutdown and close the connection to the client
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);

                // Check the return value and take appropriate action
                // The recv handling cases
                if (value < 0) {
                    printf("Cleaning everything and close connection...\n\n");
                    exit(EXIT_FAILURE);
                } else {
                    printf("Shutdown and close connection to client\n\n");
                    exit(EXIT_SUCCESS);
                }

            }
        
        }
    }
}
