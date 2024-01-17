# Log server implementation

## Project description

The goal of this project is to implement a "log server" – a server that logs data received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility.

## Executable
### Client executable
The client executable is equipped with the following command-line options:

- **IP address and listening port of the server:** users can specify the server's IP address and listening port to establish a connection as commands argument of executable.
### Example
<pre>./client localhost 9000</pre>
or
<pre>./client 127.0.0.1 9000</pre>
The client will connect to **localhost:9000** or **127.0.0.1:9000**
### Server executable

The server executable accepts the following command-line options:

- **Listening port and log file directory:** users can define the port and log file directory on which the server listens for incoming client connections and save the logfiles. The log file will be saved in the directory specified but in the same current working directory. 
### Example
<pre>./server 9000 logfile</pre>
The logfiles will be saved in **current_working_dir/logfile**
## Default configuration file
The client and server have default configuration files that are used when the user doesn't write anything as command-line options. In the configuration file of client there are the default IP address and listening port. For the server there are the default listening port and log file directory. 
### Client configuration file
The client configuration file is stored at **config/config_client** and this is the content
<pre>
ADDRESS localhost
PORT 8080
</pre>
### Server configuration file
The server configuration file is stored at **config/config_server** and this is the content
<pre>
PORT 8080
LOGPATH ../../log
</pre>
## Shutdown mechanism
### Server

The server is designed to shut down when it receives a user-selected signal from the command line. Pressing CTRL+C will gracefully terminate the server by SIGINT signal handled correctly.

### Client

The client is programmed to shut down gracefully when it receives a user-selected signal, EOF (CTRL+D) or CTRL+C from the command line. In both cases, the client correctly terminate because the two cases are handled correctly.
# System requirements

Before compiling and running the log server and client, ensure that your system meets the following requirements:
- **Operating System:** Ubuntu 22.04.1 operative system.
- **Compiler:** GCC (GNU Compiler Collection) version 11.4.0.
  - Verify your GCC version using the following command:
    ```bash
    gcc --version
    ```
  - If GCC is not installed or an older version is present, download and install the latest version from the [official GCC website](https://gcc.gnu.org/).

# Compilation and execution

To compile and run the server and client, use the provided `autorun.sh` script. The script handles compilation, server execution and client execution, ensuring a smooth and efficient workflow. It starts the server and two clients with default configuration.

### Compilation

<pre>./autorun.sh</pre>

# Test
The tests to be done are the following:
1. Server startup (start the server only)
2. Client connect (after (1), record the connection on the log file)
3. Client message (after (1), let the client send a message and the server succesfully logs it)
4. Second client message (after (3), start another client that sends a new message to the server)
5. Client shutdown (after (4), let the client succesfully close the connection and the server log the closing)
6. Server shutdown (after (5), let the server succesfully stop, after recording the order of shutdown).

**autorun.sh** script does the 1, 2 and 3 test cases.
