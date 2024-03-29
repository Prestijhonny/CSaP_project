# Log server project 
A simple client/server application facilitating communication between clients and servers.
# About project
The goal of this project is to implement a "log server" – a server that logs data received from clients into a log file. The client and server executables are designed to be configurable through command-line options, with default values stored in a configuration file for flexibility. More information about the project can be found in the [Report.pdf](Report.pdf)
 file.

# System requirements

Before compiling and running the server and client, ensure that your system meets the following requirements:
- **Operating System:** Ubuntu 22.04.1.
  - Verify the correct OS's version using the following command:

  
    <pre>uname -a </pre> 
- **Compiler:** GCC (GNU Compiler Collection) version 11.4.0.
  - Verify your GCC version using the following command:


    <pre>gcc --version</pre>
  - If GCC is not installed or an older version is present, download and install the latest version from the [official GCC website](https://gcc.gnu.org/).
# Features
The main functions of the program are the following:
- **Server** booting and listening in a default or specified port
- **Server** records the events that happens
- **Client** can send messages to server by a stateful connection

# Structure of project's files
This is an example where the server has started by default conf. Furthermore the two executables (client and server) will be saved in **client/src** and **server/src** paths.
This is a tree representation of project's files:

<pre>CSaP_project
├── client
│   ├── include
│   │   └── client.h
│   └── src
│       ├── client
│       └── client.c
├── server
│   ├── include
│   │   └── server.h
│   └── src
│       ├── server
│       └── server.c
├── config
│   ├── config_client
│   └── config_server
├── log  
├── autorun.sh
├── README.md
├── Makefile
└── Report.pdf</pre>

# Compilation and execution

The `autorun.sh` script compile and run the server and client. It handles compilation, server execution and client execution, ensuring a smooth and efficient workflow. It boots the server and two clients with default configuration. 

<pre>./autorun.sh</pre>

More specifically, it starts three terminals where one of them is the server and the other two are the client that automatically connects to server.
