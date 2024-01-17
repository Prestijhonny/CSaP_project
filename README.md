# Log server project 
A simple client/server application facilitating communication between clients and servers.
# About project
The goal of this project is to implement a "log server" – a server that logs data received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility.

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

To compile and run the server and client, use the provided `autorun.sh` script. The script handles compilation, server execution and client execution, ensuring a smooth and efficient workflow. It boots the server and two clients with default configuration.

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

The `autorun.sh` script only boots the server, client 1, and client 2. The other test cases must be run by the user.
