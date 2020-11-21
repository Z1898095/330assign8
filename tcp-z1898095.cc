/* Brian McCarthy
 * CSCI 330 Section 1 - UNIX and Network Programming
 * Assignment 8 - TCP Programming with Sockets -- Simplified HTTP Server
 * November 20, 2020
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int port;
char* root_directory;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        perror("Requires two computer arguments");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    root_directory = argv[2];
    
    printf("Port is: %d | Root dir is: %s\n", port, root_directory);

    // Server initialization
    int sock;

    // Create the TCP socket
    /*  socket(domain, type, protocol)
        domain:
            AF_INET (ipv4)
            AF_INET6 (ipv6)
        type:
            SOCK_STREAM (TCP)
            SOCK_DGRAM (UDP)
        protocol:
            0 for Internet Protocol (IP)
        
        returns -1 on failure
    */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_struct;                 // structure for address of server
    // Construct the server sockaddr_in structure
    memset(&server_struct, 0, sizeof(server_struct)); // Clear struct 
    server_struct.sin_family = AF_INET;               // Internet/IP 
    server_struct.sin_addr.s_addr = INADDR_ANY;       // Any IP address 
    server_struct.sin_port = htons(port);             // server port 

    // Bind the socket
    /* int bind(int sockfd, const struct sockaddr *addr, 
                          socklen_t addrlen);
    */
    serverlen = sizeof(server_struct);
    if (bind(sock, (struct sockaddr *) &server_struct, serverlen) < 0)
    {
        perror("Failed to bind server socket");
        exit(EXIT_FAILURE);
    }

    /* int listen(int sockfd, int backlog); */
    // listen: make socket passive,
    // set length of queue
    // listen() sets the server socket to passive mode, making it wait for client connections
    if (listen(sock, 64) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    print("Waiting for client connections.\n");
    // Accept client connections forever
    while (int newSock = accept(sock, (struct sockaddr *) &server_struct, sizeof(server_struct)))
    {
        // fork on new client connection
        int pid = fork(); // pid == 0 for child, child pid for parent
        if (pid == 0) {
            printf("Client connection created. socket = %d, pid = %d\n", newSock, getpid());
            printf("Waiting for client message:\n");

            // read a message from the client
            char message_buffer[256];
            if ((received_len = read(newSock, message_buffer, 256)) < 0)
            {
                perror("Failed to receive message");
                shutdown(newSock, SHUT_WR);
                exit(EXIT_FAILURE);
            }

            printf("Client message received:\n %s\n", message_buffer);

            // write the message back to client
            if (write(newSock, message_buffer, received_len) != received_len)
            {
                perror("Mismatch in number of bytes");
                shutdown(newSock, SHUT_WR);
                exit(EXIT_FAILURE);
            }

            shutdown(newSock, SHUT_WR);
        }
    }
}