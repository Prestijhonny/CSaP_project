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
#define NUM_LOGFILES 5


void int_handler(int signalNum);
int readConfFile(int *PORT, char LOGPATH[]);
int createDir(char LOGPATH[]);
void findLastModifiedFile(char *path);
int countFilesInDirectory(char *path);
int handleClientConn(int clientSocket, char clientAddr[], int intPortOfClient);