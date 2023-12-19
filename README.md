# VIT CommSuite - Socket Programming in C
<a href="https://github.com/AtharvaKulkarniIT/VIT-CommSuite-in-C"><img src="https://sloc.xyz/github/AtharvaKulkarniIT/VIT-CommSuite-in-C" alt="LOC"/></a>
<a href="https://github.com/AtharvaKulkarniIT/VIT-CommSuite-in-C"><img src="https://visitor-badge.laobi.icu/badge?page_id=AtharvaKulkarniIT.VIT-CommSuite-in-C" alt="Visitors"/></a>
<a href="https://github.com/AtharvaKulkarniIT/VIT-CommSuite-in-C/stargazers"><img src="https://img.shields.io/github/stars/AtharvaKulkarniIT/VIT-CommSuite-in-C" alt="Stars Badge"/></a>

## Features

- Multithreading: The server can handle multiple client connections concurrently using threads, ensuring efficient communication.
- File Handling: Clients can send and receive files from the server, which are stored in the "ServerFiles" directory.
- Secure Admin Authentication: The server supports an admin mode that requires a password for authentication.

## Prerequisites

- A C compiler (e.g., GCC)
- A machine running a Unix-based operating system (Linux, macOS)
- Basic knowledge of C programming and socket programming concepts

## Server

The server code (`server.c`) provides the following features:

- Accepts connections from multiple clients.
- Supports admin functions, such as setting an admin password and managing admin status.
- Handles file transfers from clients.
- Stores received files in a directory structure within the "ServerFiles" folder.

### Compilation Steps for Server

To compile and run the server code, follow these steps:

1. Open a terminal.
2. Navigate to the project directory.
3. Compile the server code using the following command:

   ```shell
   gcc server.c -o server -lpthread
   ```

4. Run the server:

   ```shell
   ./server [port_number] [ip_address]
   ```

   By default, the server will listen on port 8080 and the local IP address. You can specify the port and IP address as arguments if needed.

## Client

The client code (`client.c`) is used by individual clients to connect to the server and communicate with other clients. Clients can send text messages and request file transfers.

### Compilation Steps for Client

To compile and run the client code, follow these steps:

1. Open a terminal.
2. Navigate to the project directory.
3. Compile the client code using the following command:

   ```shell
   gcc client.c -o client
   ```

4. Run the client:

   ```shell
   ./client [server_ip] [port_number]
   ```

   Specify the server's IP address and port number as arguments to connect to the server.

## Sample Files

Two sample files are provided in the "SampleFiles" folder within this repository. These files can be used for testing file transfer functionality.

- `notes.txt`: A text file with sample content.
- `program.cpp`: A sample cpp file.
- `output.png`: A sample output image file.

## Usage

To run the project effectively, open at least two client terminals: one for a teacher and one for a student. You can open more client terminals as needed.

**Server Terminal:**

In the server terminal, compile and run the server code to handle incoming client connections.

```shell
# Compile and run the server
gcc server.c -o server -lpthread
./server [port_number] [ip_address]
```

**Teacher Client Terminal:**

This client can use the `set-admin` command to become an admin with the password `vit@2023`, and it can also use the `logout` command to relinquish admin status.

```shell
# Compile and run the teacher client
gcc client.c -o client
./client [server_ip] [port_number]

# After connecting, you can use the following commands:
set-admin     # To become an admin with the password "vit@2023"
logout        # To log out as an admin
send          # To send files to students
exit          # To exit the client
```

**Student Client Terminal:**

This client can communicate with the server and, send files using the `send` command to the admin teacher. It can also use the `exit` command to exit the client.

```shell
# Compile and run the student client
gcc client.c -o client
./client [server_ip] [port_number]

# After connecting, you can use the following commands:
send    # To send files to an admin teacher only (if available)
exit    # To exit the client
```

By opening multiple teacher and student client terminals, you can simulate a classroom environment for testing and interaction.

Please note that you should replace `[server_ip]` and `[port_number]` with the IP address and port where your server is running. Make sure the server is up and running before you start the clients.

## Easiest compilation and running steps

Follow these simplified steps to compile and run the VIT CommSuite project:

1. Compile the server code:
   ```shell
   gcc server.c -o server
   ```

2. Run the server:
   ```shell
   ./server
   ```

3. Compile the client code:
   ```shell
   gcc client.c -o client
   ```

4. Run the client:
   ```shell
   ./client
   ```

By following these steps, you can quickly compile and run the server and client applications without specifying port numbers or IP addresses. The server will listen on the default port 8080, and the client will connect to the local IP address. You can customize these settings by providing command-line arguments as mentioned in the detailed README.

Make sure to open multiple client terminals to simulate a classroom environment for testing and interaction.

## Contributions

Contributions to this project are welcome. If you'd like to enhance the functionality or fix any issues, please feel free to submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
