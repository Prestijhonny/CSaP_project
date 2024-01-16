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
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>
#include <semaphore.h>
#include <sys/wait.h>
#define TRUE 1
#define LOG "LOG"
#define CONFIG_PATH "../../config/config_server"
#define LOGFILE_THRESHOLD 1000
#define MAX_PATH 1024
#define NUM_LOGFILES 4

sem_t sem;
int sockfd, clientSocket;
pid_t PPID;
FILE *logFile;
char LOGPATH[MAX_PATH];

void createNewFilename(char path[]);
void handler(int signo);
void registerServerShutdown();
void findLastModifiedFile(char *path);
int deleteLeastRecentlyFile(char *directory_path);
int readConfFile(int *PORT);
int createDir();
int countFilesInDirectory(char *path);
int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient);
int countNumberOfCharacters(char path[]);
FILE * getFileDescriptor(int sizeOfMessage);

void cleanupAndExit() {
    sem_close(&sem);
    sem_destroy(&sem);
    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
}

FILE * getFileDescriptor(int sizeOfMessage)
{
    // Count the number of files in the log directory
    int numFiles = countFilesInDirectory(LOGPATH);
    char pathToNewFile[MAX_PATH];

    // Use pathToNewFile as a placeholder for the path to the file without modifying LOGPATH
    strcpy(pathToNewFile, LOGPATH);

    // If there are zero files, create the first one
    if (numFiles == 0) {
        createNewFilename(pathToNewFile);
    } else if (numFiles <= NUM_LOGFILES) {
        // If there is at least one file, open the most recently used
        // This function will fill path with the name of the latest file used
        findLastModifiedFile(pathToNewFile);

        // Count the number of characters in the last modified file
        int numberOfCharacters = countNumberOfCharacters(pathToNewFile);

        // If the number of characters written on the logfile and the size of the message exceed LOGFILE_THRESHOLD
        if ((numberOfCharacters + sizeOfMessage) >= LOGFILE_THRESHOLD) {
            // If the number of files reached the maximum number of log files, delete the oldest logfile
            if (numFiles == NUM_LOGFILES) {
                // Delete the least recently used file
                if (deleteLeastRecentlyFile(LOGPATH) == -1) {
                    printf("Error deleting the least recently used file. The client will be kicked out. "
                           "Try to restart the server or delete the file manually.\n");
                    sem_post(&sem);
                    cleanupAndExit();
                    exit(EXIT_FAILURE);
                }
            }

            // Reset pathToNewFile to contain the log path, then create a new filename
            strcpy(pathToNewFile, LOGPATH);
            createNewFilename(pathToNewFile);
        }
    }

    // Open the log file for append
    FILE *lf = fopen(pathToNewFile, "a");
    if (lf == NULL) {
        printf("Error opening file\n");
        sem_post(&sem);
        cleanupAndExit();
        exit(EXIT_FAILURE);
    }

    return lf;
}

void registerServerShutdown()
{
    char shutdownServer[64] = "The server is shutting down at " ;
    // Get current time
    time_t currentTime;
    time(&currentTime);
    // Convert time to string representation
    char *timeString = ctime(&currentTime);
    strcat(shutdownServer, timeString);
    strcat(shutdownServer, "\n");  
    sem_wait(&sem);
    FILE *fp = getFileDescriptor(strlen(shutdownServer));
    write(fileno(fp), shutdownServer, strlen(shutdownServer));
    fclose(fp);
    sem_post(&sem);
}

void createNewFilename(char path[])
{
    time_t rawtime;
    struct tm *timeinfo;
    char nameFile[MAX_PATH];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    // Format the date and time
    strftime(nameFile, sizeof(nameFile), "%Y%m%d_%H_%M_%S.txt", timeinfo);
    strcat(path, "/");
    strcat(path, LOG);
    strcat(path, nameFile);
}

int deleteLeastRecentlyFile(char *directory_path)
{
    DIR *dir = opendir(directory_path);

    if (dir == NULL)
    {
        printf("Error opening directory");
        return -1;
    }
    // Get the current time 
    time_t lessRecently = time(NULL);
    char lessRecentlyFile[MAX_PATH];
    char file_path[MAX_PATH];
    struct dirent *entry;
    struct stat file_stat;
    // Search for a file that is different from "." and ".."
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {

            memset(file_path, 0, sizeof(file_path));
            strcpy(file_path, directory_path);
            strcat(file_path, "/");
            strcat(file_path, entry->d_name);

            // stat is needed for retrieve file information and if it returns 0, it means that it succeed
            if (stat(file_path, &file_stat) == 0)
            {
                // Get time of last modification of file and compare it to lessRecently
                if (file_stat.st_mtime < lessRecently)
                {
                    lessRecently = file_stat.st_mtime;
                    // Clean and fill the string
                    memset(lessRecentlyFile, 0, sizeof(lessRecentlyFile));
                    strcpy(lessRecentlyFile, file_path);
                }
            }
            else{
                printf("Error getting file information");
                closedir(dir); 
                return -1;
            }
        }
    }

    if (remove(lessRecentlyFile) != 0){
        printf("Error deleting last log file\n");
        closedir(dir); 
        return -1;
    }
    closedir(dir); 
}

// Function to find last modified log file
void findLastModifiedFile(char *path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    time_t latestModTime = 0;
    char latestModFileName[MAX_PATH];
    int nFile = 0;

    if ((dir = opendir(path)) == NULL)
    {
        printf("Error opening directory\n");
        sem_post(&sem);
        cleanupAndExit();
        exit(EXIT_FAILURE);
    }
    char file_path[MAX_PATH];
    // Read the entry of the dir
    while ((entry = readdir(dir)) != NULL)
    {
        // If the file is not equal to current dir "." or parent dir ".." then analayze the file 
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)){
            // Create the file path with the name of the file found
            memset(file_path, 0, sizeof(file_path));
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, entry->d_name);

            if (stat(file_path, &fileStat) == 0){
                if (fileStat.st_mtime > latestModTime){
                    latestModTime = fileStat.st_mtime;
                    memset(latestModFileName, 0, sizeof(latestModFileName));
                    strcpy(latestModFileName, entry->d_name);
                }
            }else{
                printf("Error getting file information");
                sem_post(&sem);
                cleanupAndExit();
                exit(EXIT_FAILURE);
            }
        }
    }
    // Copy the latest modified file path in path variable, it means that the path variable passed to the function
    // is changed directly without need to return it
    strcat(path, "/");
    strcat(path, latestModFileName);
    
    closedir(dir);
}

// This handler manage SIGINT signal (CTRL+c pressed) and SIGUSR1 signal is sent when a child processes cannot delete
// the oldest file in the log directory
void handler(int signo) {
    pid_t currentPid = getpid();

    if (currentPid != PPID) {
        // Child process
        int filDes = fileno(logFile);
        // Check if the file descriptor is opened by the child processes and if the file is opened in append mode, then close the file descriptor
        if (filDes >= 0 && (fcntl(filDes, F_GETFL) & O_APPEND))
            fclose(logFile);
        sem_post(&sem);
        sem_close(&sem);
        sem_destroy(&sem);
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        exit(EXIT_SUCCESS);
    }else{
        // Parent process
        if (signo == SIGINT) 
            printf("\nSIGINT signal received, shutdown and close socket for all processes\n");

        registerServerShutdown();

        sem_post(&sem);
        sem_close(&sem);
        sem_destroy(&sem);
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        // Wait until there aren't more child processes
        while (wait(NULL) != -1);

        exit(EXIT_SUCCESS);
    }
}

// Read data from conf file when server starts
int readConfFile(int *PORT)
{
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (fp == NULL)
    {
        printf("Error to open config file");
        return -1;
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

int createDir()
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
        printf("Unable to open log directory, closing client connection\n");
        sem_post(&sem);
        cleanupAndExit();
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

int countNumberOfCharacters(char path[])
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL){
        printf("Error to open the logfile, closing client connection\n");
        sem_post(&sem);
        cleanupAndExit();
        exit(EXIT_FAILURE);
    }
    // Positioning the pointer at end of file
    fseek(fp, 0, SEEK_END);
    // Get the current position of pointer with respect to the beginning of the file
    int numberOfCharaters = ftell(fp);
    // Positioning the pointer at start of file
    fseek(fp, 0, SEEK_SET);
    return numberOfCharaters;
}

int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient)
{
    char outMessage[2048];
    char *timeString;
    char logAddress[64];
    char portClient[16];
    char message[MAX_PATH];
    ssize_t bytesReceived;

    while (TRUE)
    {
        memset(outMessage, 0, sizeof(outMessage));
        // Get current time
        time_t currentTime;
        time(&currentTime);
        // Convert time to string representation
        timeString = ctime(&currentTime);
        strcpy(outMessage, timeString);
        strcpy(logAddress, "Client address: ");
        strcat(logAddress, clientAddr);
        strcat(logAddress, ":");
        memset(portClient, 0, sizeof(portClient));
        // Convert integer to string
        snprintf(portClient, sizeof(portClient), "%d", intPortOfClient);

        strcat(logAddress, portClient);
        strcat(outMessage, logAddress);
        strcat(outMessage, "\n");
        strcat(outMessage, "Client message: ");
        // Clean message string before using it
        memset(message, 0, sizeof(message));
        // Number of bytes received from client socket and store it in message string
        bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        // Handling recv error
        if (bytesReceived == -1){
            printf("Error to receive data from socket\n");
            return -1;
        }else if (bytesReceived == 0){
            // It means that the client has disconnected
            printf("The client %s:%s has disconnected\n", clientAddr, portClient);
            sem_wait(&sem);
            char disconnectedClient[MAX_PATH];
            snprintf(disconnectedClient,sizeof(disconnectedClient), "The client %s:%s has disconnected\n", clientAddr, portClient);
            logFile = getFileDescriptor(strlen(disconnectedClient));
            write(fileno(logFile),disconnectedClient,strlen(disconnectedClient));
            fclose(logFile);
            sem_post(&sem);
            
            return 0;
        }

        strcat(outMessage, message);
        strcat(outMessage, "\n");

        sem_wait(&sem);
        // I use this feature via semaphore, so i'm sure processes access the file one at a time
        logFile = getFileDescriptor(strlen(outMessage));
        write(fileno(logFile), outMessage, strlen(outMessage));
        fclose(logFile);
        sem_post(&sem);
    }
}