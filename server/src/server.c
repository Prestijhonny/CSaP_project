#include "../include/server.h"

int sockfd;
FILE * logFile;
pid_t PPID;
sem_t sem;

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
    
    // Install SIGINT signal: manage ctrl+c action by int_handler function
    signal(SIGINT, int_handler);

    // ---------------------------------------------------------------------------
    // Create log file at startup
    char path[1024];
    strcpy(path, LOGPATH);
    strcat(path, "/");
    
    // There are zero files 
    if (countFilesInDirectory(path) == 0){
        time_t rawtime;
        struct tm *timeinfo;
        char nameFile[80];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        // Format the date and time
        strftime(nameFile, sizeof(nameFile), "%Y%m%d_%H_%M_%S.txt", timeinfo);
        strcat(path, LOG);
        strcat(path, nameFile);
       
        // 1) All'accensione del server se non ci sono log file crearne uno e scrivere su quello altrimenti scrivere sul piu` recente su cui e` stato scritto
        
        // 2) Verificare se quando si scrive su un file e` stata superata una soglia LOGFILE_THRESHOLD di lunghezza, se si, allora, creare un nuovo file e scrivere su quello; contenstualmente se il numero dei file di log supera un certo limite, diciamo NUM_LOGFILE, il file di log piu' vecchio deve essere cancellato

        // I created FILE *log globally because i want to make it usable in sigint handler function
        logFile = fopen(path, "a");
            if (logFile == NULL){
                printf("Error opening file\n");
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
    }
    // ---------------------------------------------------------------------------
    
    printf("-------------------------------\n");
    printf("| Server started successfully |\n");
    printf("-------------------------------\n");
    // Get current time
    time_t currentTime;
    time(&currentTime);
    // Convert time to string representation
    char *timeString = ctime(&currentTime);
    printf("\nDate and time: %s\n",timeString);
    sem_init(&sem,0,1);
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
            printf("A client has connected, accepted connection from %s:%d\n", clientAddr, intPortOfClient);

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


                if (handleClientConn(clientSocket, clientAddr, intPortOfClient) < 0)
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
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient)
{
    
    while (TRUE){
        char out[2048];
        memset(out, 0, sizeof(out));
        // Get current time
        time_t currentTime;
        time(&currentTime);
        // Convert time to string representation
        char *timeString = ctime(&currentTime);
        strcpy(out, timeString);
        char logAddress[1024] = "Client address: ";
        strcat(logAddress, clientAddr);
        strcat(logAddress, ":");
        char portClient[16];
        snprintf(portClient, sizeof(portClient), "%d", intPortOfClient);
        strcat(logAddress, portClient);
        strcat(out, logAddress);
        strcat(out, "\n");
        char message[1024];
        strcat(out, "Client message: ");
        // Clean message string VERY IMPORTANT to make it works correctly
        memset(message, 0, sizeof(message));
        // Number of bytes received from clien socket and store it in message string 
        ssize_t bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        // Handling recv error
        if (bytesReceived == -1){
            printf("Error to receive data from socket\n");
            return -1;
        }else if (bytesReceived == 0){
        // It means that the client has disconnected
            printf("The client %s:%d has disconnected\n",clientAddr,intPortOfClient);
            return 0;
        }
        
        strcat(out, message);
        strcat(out, "\n");
        
        sem_wait(&sem);

        write(fileno(logFile),out ,strlen(out));

        sem_post(&sem);
    }

    
}


// Handler for SIGINT signal
void int_handler(int signo)
{
    sem_close(&sem);
    // Only parent process use this code
    if (getpid() == PPID){
        printf("\nSIGINT signal received, shutdown and close socket for all processes\n");
        sem_destroy(&sem);
    }
    
    if (logFile != NULL)
        fclose(logFile);

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

// Read data from conf file
int readConfFile(int *PORT, char LOGPATH[])
{
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (fp == NULL)
    {
        printf("Error to open config file");
        exit(EXIT_FAILURE);
    }
    char line[50];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (sscanf(line, "PORT %d", PORT) == 1)
            printf("Listening port: %d\n", *PORT);
        else if (sscanf(line, "LOGPATH %s", LOGPATH) == 1)
            printf("Log file path: %s\n", LOGPATH);
    }
    fclose(fp);
    return 0;
}

int createDir(char LOGPATH[])
{
    // If LOGPATH does not exists then create it
    struct stat st;
    if (stat(LOGPATH, &st) == -1)
    {
        if (mkdir(LOGPATH, 777) == -1)
        {
            printf("Error to create a dir");
            return -1;
        }
    }

    return 0;
}

// Function to find last modified log file
void findLastModifiedFile(char *path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    time_t latestModTime = 0;
    char latestModFileName[256] = "";
    int nFile = 0;
    dir = opendir(path);

    if (dir == NULL)
    {
        printf("Error opening directory\n");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
    {

        char filePath[512];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
        {

            if (stat(filePath, &fileStat) == 0)
            {
                if (fileStat.st_mtime > latestModTime)
                {
                    latestModTime = fileStat.st_mtime;
                    strncpy(latestModFileName, entry->d_name, sizeof(latestModFileName));
                }
            }
            else
            {
                printf("Error getting file information");
                exit(EXIT_FAILURE);
            }
        }
    }
    strcat(path, "/");
    strcat(path, latestModFileName);
    closedir(dir);
}

int countFilesInDirectory(char *path)
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    // Open the directory
    dir = opendir(path);

    // Check if the directory was successfully opened
    if (dir == NULL)
    {
        perror("Unable to open directory");
        exit(EXIT_FAILURE);
    }

    // Count files in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
            count++;
    }

    // Close the directory
    closedir(dir);

    return count;
}