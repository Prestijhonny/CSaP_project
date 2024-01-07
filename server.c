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
#include <dirent.h>
#define TRUE 1
#define LOG "LOG"
#define CONFIG_PATH "config/config_server"
#define N_LOGFILE 10
#define LOGFILE_THRESHOLD 65536 // byte

int sockfd;
void int_handler(int signalNum);
int readConfFile(int PORT, char LOGPATH[]);
int createDir(char LOGPATH[]);
void findLastModifiedFile(char *path);
int countFilesInDirectory(char *path);
int handleClientConn(int clientSocket, FILE *log, char clientAddr[]);

int main(int argc, char *argv[])
{
    char LOGPATH[1024];
    int PORT;
    struct sockaddr_in server, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // Default values for port and log path
    if (argc == 1)
    {
        // Read default values from config server file
        if (readConfFile(PORT, LOGPATH) < 0)
            exit(EXIT_FAILURE);
        // Values passed from command line
    }
    else if (argc == 3)
    {
        PORT = atoi(argv[1]);
        strcpy(LOGPATH, argv[2]);
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
    printf("-------------------------------\n");
    printf("| Server started successfully |\n");
    printf("-------------------------------\n");
    // Install SIGINT signal: manage ctrl+c action by int_handler function
    signal(SIGINT, int_handler);

    // ---------------------------------------------------------------------------
    // Create log file at startup
    char path[1024];
    strcpy(path, LOGPATH);
    strcat(path, "/");
    strcat(path, LOG);
    // Da modificare , momentanemente lasciamo la gestione dei file di log a quando il prof risponde
    time_t rawtime;
    struct tm *timeinfo;
    char nameFile[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    // Format the date and time
    strftime(nameFile, sizeof(nameFile), "%Y%m%d_%H_%M_%S.txt", timeinfo);
    strcat(path, nameFile);
    // ---------------------------------------------------------------------------

    // Client socket where to read data
    int clientSocket;
    while (TRUE)
    {
        // Accept function extract the first connection on the queue of pending connections
        // pcreate a new socket with the same socket type protocol and address family as the specified socket
        if ((clientSocket = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            printf("Error accepting client connection...\n");

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

            // 0) Il processo figlio dovrebbe durare per affinche` il client e` connesso

            // 1) Verificare il numero di logfile, se sono N_LOGFILE allora appendere i dati al file di log piu` recente, altrimenti, creare nuovo log file

            // 2) Prima di eseguire un operazione di scrittura verificare sempre se e` stata raggiunta la LOGFILE_THRESHOLD, in caso positivo, cancellare il logfile piu' vecchio, crearne uno nuovo e scrivere su quello

            FILE *log = fopen(path, "a");
            if (log == NULL)
            {
                printf("Error opening file\n");
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
                exit(EXIT_FAILURE);
            }

            char clientAddr[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(client_addr.sin_addr), clientAddr, sizeof(clientAddr)) == NULL)
            {
                printf("Error converting client address to string");
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
                exit(EXIT_FAILURE);
            }

            // Pensare a due messaggi migliori
            if (handleClientConn(clientSocket, log, clientAddr) < 0){
                printf("The server will clean everything for this connection...\n");
            }else{
                printf("The server will clean \n");
            }

            fclose(log);
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

int handleClientConn(int clientSocket, FILE *log, char clientAddr[])
{
    while (TRUE){
                char out[2048];
                // Get current time
                time_t currentTime;
                time(&currentTime);
                // Convert time to string representation
                char *timeString = ctime(&currentTime);
                strcpy(out, timeString);

                char logAddress[1024] = "Client address: ";
                strcat(logAddress, clientAddr);
                strcat(out, logAddress);
                strcat(out, "\n");
                char message[1024];
                strcat(out, "Client message: ");
                ssize_t bytesReceived = recv(clientSocket, message, sizeof(message), 0);
                if (bytesReceived == -1){
                    printf("Error to receive data from socket\n");
                    return -1;
                }else if (bytesReceived == 0){
                    printf("The client has disconnected\n");
                    return 0;
                }
                strcat(out, message);
                strcat(out, "\n");

                struct flock fl;
                fl.l_type = F_WRLCK; // Exclusive write lock
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0; // Lock the entire file
                
                int logFd = fileno(log);

                if (fcntl(logFd, F_SETLKW, &fl) == -1)
                {
                    printf("Error locking file");
                    return -1;
                }
                
                fwrite(out, 1, strlen(out), log);

                fl.l_type = F_UNLCK;
                if (fcntl(logFd, F_SETLKW, &fl) == -1)
                {
                    printf("Error unlocking file");
                    return -1;
                }
                
            }
}


// Handler for SIGINT signal
void int_handler(int signo)
{
    printf("\nSIGINT signal received, shutdown and close socket\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

// Read data from conf file
int readConfFile(int PORT, char LOGPATH[])
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

        if (sscanf(line, "PORT %d", &PORT) == 1)
            printf("Listening port: %d\n", PORT);
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