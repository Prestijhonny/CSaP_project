# Log server implementation

## Project description

The goal of this project is to implement a "log server" – a server that logs inputs received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility.

## Client executable

The client executable is equipped with the following command-line options:

- **IP address and listening port of the server:** users can specify the server's IP address and listening port to establish a connection as commands argument of executable.
### Example
<pre>./client localhost 9000</pre>
or
<pre>./client 127.0.0.1 9000</pre>
The client will connect to **localhost:9000** or **127.0.0.1:9000**
## Server executable

The server executable accepts the following command-line options:

- **Listening port and log file directory:** users can define the port and log file directory on which the server listens for incoming client connections and save the logfiles. The log file will be saved in the directory specified but in the same current working directory. 
### Example
<pre>./server 9000 logfile</pre>
The logfiles will be saved in **current_working_dir/logfile**
## Default configuration file
The client and server have default configuration files that are used when the user doesn't write anything as command-line options. In the configuration file of client there are the default IP address and listening port. For the server there are the default listening port and log file directory. 
### Client configuration file
ADDRESS localhost
PORT 8080
### Server configuration file
PORT 8080
LOGPATH log
## Server Shutdown Mechanism

The server is designed to shut down when it receives a user-selected signal or a quit command from the command line. For example, pressing CTRL+C will gracefully terminate the server.

## Client Shutdown Mechanism

The client is programmed to shut down gracefully when it receives a user-selected signal, EOF (CTRL+D), or a quit command from the command line.

# Compilation and Execution

To compile and run the server and client, use the provided `autorun.sh` script. The script handles compilation, server execution, and client execution, ensuring a smooth and efficient workflow.

### Compilation

```bash
./autorun.sh compile
