#include "../include/server.h"

extern sem_t sem;
extern int sockfd;
extern FILE * logFile;
extern pid_t PPID;

int main(int argc, char *argv[])
{
    char LOGPATH[1024];
    int PORT;
    struct sockaddr_in server, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Get parent process pid
    PPID = getpid();
    // Default values for port and log path
    if (argc == 1)
    {
        // Read default values from config server file
        if (readConfFile(&PORT, LOGPATH) < 0)
            exit(EXIT_FAILURE);
        
    }
    else if (argc == 3) // Values passed from command line
    {
        PORT = atoi(argv[1]);
        strcpy(LOGPATH, "../../");
        strcat(LOGPATH, argv[2]);
        printf("Listening port: %d\n", PORT);
        if (createDir(LOGPATH) < 0)
        {
            exit(EXIT_FAILURE);
        }
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
    // Install SIGUSR1 signal: manage the case when file has exceeded the threshold in terms of characters
    // signal(SIGUSR1, handler);
    char path[1024];
    strcpy(path, LOGPATH);
    strcat(path, "/");
    
    int numFile = countFilesInDirectory(path);
    // If there are zero files, i will create the first
    if (numFile == 0){
        printf("Debug before %s\n", path);
        createNewFilename(path);
        printf("Debug after %s\n", path);
        getchar();
        // 1) All'accensione del server se non ci sono log file crearne uno e scrivere su quello altrimenti scrivere sul piu` recente su cui e` stato scritto
        
        // 2) Verificare se quando si scrive su un file e` stata superata una soglia LOGFILE_THRESHOLD di lunghezza, se si, allora, creare un nuovo file e scrivere su quello; contenstualmente se il numero dei file di log supera un certo limite, diciamo NUM_LOGFILE, il file di log piu' vecchio deve essere cancellato

        
    }else if (numFile <= NUM_LOGFILES){ 
        // If there is at least one file, i will open the most recently used 
        // This function will fill path with the name of the latest file used
        findLastModifiedFile(path);
    }
    
    // I created FILE *log globally because i want to make it usable in sigint handler function
    // I get the file descriptor by the parent process because every child process created will inherits this file descriptor
    logFile = fopen(path, "a");
    if (logFile == NULL){
        printf("Error opening file\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    
    printf("-------------------------------\n");
    printf("| Server started successfully |\n");
    printf("-------------------------------\n");
    // Get current time
    time_t currentTime;
    time(&currentTime);
    // Convert time to string representation
    char *timeString = ctime(&currentTime);
    printf("\nDate and time: %s\n",timeString);
    
    // Unamed semaphore
    if (sem_init(&sem,0,1)  == -1){
        printf("Error initializing semaphore\n");
        fclose(logFile);
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Client socket where to read data
    int clientSocket;
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
            
            int intPortOfClient = ntohs(client_addr.sin_port);
            printf("A client has connected, accepted connection from %s:%d\n\n", clientAddr, intPortOfClient);

            pid_t pid = fork();

            if (pid == -1)
            {
                printf("Error creating child process\n");
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
            }
            else if (pid == 0)
            {
                // Child process
                // Close the socket created by server to save resources
                close(sockfd);


                if (handleClientConn(clientSocket, clientAddr, intPortOfClient, path) < 0)
                    printf("Error: cleaning everything\n");
                else
                    printf("Shutdown and close connection\n\n");
                
                sem_close(&sem);
                fclose(logFile);
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
                exit(EXIT_SUCCESS);
            }
        
        }
    }

    sem_destroy(&sem);
    fclose(logFile);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

