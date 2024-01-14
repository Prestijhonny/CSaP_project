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
#define TRUE 1
#define LOG "LOG"
#define CONFIG_PATH "../../config/config_server"
#define LOGFILE_THRESHOLD 1024
#define MAX_PATH 1024
#define NUM_LOGFILES 5

sem_t sem;
int sockfd;
pid_t PPID;
FILE *logFile;

void createNewFilename(char path[]);
void handler(int signo);
void checkFile(char logPath[], char out[]);
void findLastModifiedFile(char *path);
int readConfFile(int *PORT, char LOGPATH[]);
int createDir(char LOGPATH[]);
int countFilesInDirectory(char *path);
int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient, char logPath[]);
int countNumberOfCharacters(char path[]);
char *findLeastRecentlyFile(char *directory_path);


void createNewFilename(char path[])
{
    time_t rawtime;
    struct tm *timeinfo;
    char nameFile[MAXNAMLEN];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    // Format the date and time
    strftime(nameFile, sizeof(nameFile), "%Y%m%d_%H_%M_%S.txt", timeinfo);
    strcat(path, "/");
    strcat(path, LOG);
    strcat(path, nameFile);
}

int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient, char logPath[])
{

    while (TRUE)
    {
        char out[2048];
        memset(out, 0, sizeof(out));
        // Get current time
        time_t currentTime;
        time(&currentTime);
        // Convert time to string representation
        char *timeString = ctime(&currentTime);
        strcpy(out, timeString);
        char logAddress[64] = "Client address: ";
        strcat(logAddress, clientAddr);
        strcat(logAddress, ":");
        char portClient[16];
        // Convert integer to string
        snprintf(portClient, sizeof(portClient), "%d", intPortOfClient);
        strcat(logAddress, portClient);
        strcat(out, logAddress);
        strcat(out, "\n");
        char message[MAX_PATH];
        strcat(out, "Client message: ");
        // Clean message string VERY IMPORTANT to make it works correctly
        memset(message, 0, sizeof(message));
        // Number of bytes received from client socket and store it in message string
        ssize_t bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        // Handling recv error
        if (bytesReceived == -1){
            printf("Error to receive data from socket\n");
            return -1;
        }else if (bytesReceived == 0)
        {
            // It means that the client has disconnected
            printf("The client %s:%d has disconnected\n", clientAddr, intPortOfClient);
            return 0;
        }

        strcat(out, message);
        strcat(out, "\n");

        sem_wait(&sem);

        checkFile(logPath, out);

        sem_post(&sem);
    }
}

void checkFile(char logPath[], char out[])
{
    int numFile = countFilesInDirectory(logPath);
    char pathToNewFile[MAX_PATH];
    // I use pathToNewFile as a placeholder for path to the file
    strcpy(pathToNewFile, logPath); 
    printf("DEBUG before pathToNewFile %s\n",pathToNewFile);
    // If there are zero files, i will create the first
    if (numFile == 0){
        createNewFilename(pathToNewFile);
    }else if (numFile <= NUM_LOGFILES){
        // If there is at least one file, i will open the most recently used
        // This function will fill path with the name of the latest file used
        findLastModifiedFile(pathToNewFile);
        int numberOfCharacters = countNumberOfCharacters(pathToNewFile);
        // If the file has exceeded the threshold in terms of characters
        if (numberOfCharacters >= LOGFILE_THRESHOLD)
        {
            // If the number of files reached the max number of logfile, it will be deleted the oldest logfile
            if (countFilesInDirectory(logPath) == NUM_LOGFILES)
            {
                char *fileToDelete = findLeastRecentlyFile(logPath);
                if (remove(fileToDelete) == 0)
                    printf("File '%s' deleted successfully.\n", fileToDelete);
                else
                    printf("Error deleting file\n");
            }
            createNewFilename(pathToNewFile);
        }
    }
    printf("DEBUG after pathToNewFile %s\n",pathToNewFile);
    logFile = fopen(pathToNewFile, "a");
    if (logFile == NULL){
        printf("Error opening file\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    write(fileno(logFile), out, strlen(out));
    fclose(logFile);
}

char *findLeastRecentlyFile(char *directory_path)
{
    DIR *dir = opendir(directory_path);

    if (dir == NULL)
    {
        printf("Error opening directory");
        return NULL;
    }

    time_t lessRecently = time(NULL);
    char *lessRecentlyFile = NULL;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {

            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

            struct stat file_stat;
            if (stat(file_path, &file_stat) == 0)
            {
                if (file_stat.st_mtime < lessRecently)
                {
                    lessRecently = file_stat.st_mtime;
                    if (lessRecentlyFile != NULL)
                        free(lessRecentlyFile);

                    lessRecentlyFile = strdup(file_path);
                }
            }
            else
                printf("Error getting file information");
        }
    }

    closedir(dir);
    return lessRecentlyFile;
}

// Handler for SIGINT e SIGUSR1 signal
void handler(int signo)
{
    if (signo == SIGINT)
    {
        fflush(stdin);
        sem_close(&sem);
        // Only parent process use this code
        if (getpid() == PPID)
        {
            printf("\nSIGINT signal received, shutdown and close socket for all processes\n");
            sem_destroy(&sem);
        }

        /*if (logFile != NULL)
            fclose(logFile);*/

        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(EXIT_SUCCESS);
    }

}

// Read data from conf file when server starts
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
        printf("Unable to open directory");
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
        printf("Error to open file\n");
        return -1;
    }
    // Positioning the pointer at end of file
    fseek(fp, 0, SEEK_END);
    // Get the current position of pointer with respect to the beginning of the file
    int numberOfCharaters = ftell(fp);
    // Positioning the pointer at start of file
    fseek(fp, 0, SEEK_SET);
    return numberOfCharaters;
}
