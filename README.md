# Log Server Implementation

## Project Description

The goal of this project is to implement a "log server" – a server that logs inputs received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility.

### Client Executable

The client executable is equipped with the following command-line options:

- **IP Address and Listening Port of the Server:** Users can specify the server's IP address and listening port to establish a connection.
- **Shutdown Options:** The client can be terminated gracefully using user-selected signals, EOF (CTRL+D), or a quit command from the command line.

### Server Executable

The server executable accepts the following command-line options:

- **Listening Port:** Users can define the port on which the server listens for incoming client connections.
- **Log File Directory:** A directory path where the log file will be stored.

### Server Shutdown Mechanism

The server is designed to shut down when it receives a user-selected signal or a quit command from the command line. For example, pressing CTRL+C will gracefully terminate the server.

### Client Shutdown Mechanism

The client is programmed to shut down gracefully when it receives a user-selected signal, EOF (CTRL+D), or a quit command from the command line.

## Compilation and Execution

To compile and run the server and client, use the provided `autorun.sh` script. The script handles compilation, server execution, and client execution, ensuring a smooth and efficient workflow.

### Compilation

```bash
./autorun.sh compile
