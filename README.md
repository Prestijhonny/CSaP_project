# Log server project 
A simple client/server application facilitating communication between clients and servers.
# About project
The goal of this project is to implement a "log server" – a server that logs data received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility.

# System requirements

Before compiling and running the log server and client, ensure that your system meets the following requirements:
- **Operating System:** Ubuntu 22.04.1 operative system.
- **Compiler:** GCC (GNU Compiler Collection) version 11.4.0.
  - Verify your GCC version using the following command:
    <pre>gcc --version</pre>
  - If GCC is not installed or an older version is present, download and install the latest version from the [official GCC website](https://gcc.gnu.org/).
    
# Compilation and execution

The `autorun.sh` script compile and run the server and client. It handles compilation, server execution and client execution, ensuring a smooth and efficient workflow. It boots the server and two clients with default configuration. 

<pre>./autorun.sh</pre>

More specifically, it starts three terminals where one of them is the server and the other two are the client.
