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
#include "utils.h"

int main(int argc, char* argvp[]) {
    printf("hello!");
}

int asdf(int argc, char * argv[])
{
    
    int sock;

    // Create the TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in echoserver; // structure for address of server

    // Construct the server sockaddr_in structure
    memset(&echoserver, 0, sizeof(echoserver)); /* Clear struct */
    echoserver.sin_family = AF_INET;            /* Internet/IP */
    echoserver.sin_addr.s_addr = INADDR_ANY;    /* Any IP address */
    echoserver.sin_port = htons(atoi(argv[1])); /* server port */

    // Bind the socket
    serverlen = sizeof(echoserver);
    if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0)
    {
        perror("Failed to bind server socket");
        exit(EXIT_FAILURE);
    }

    // listen: make socket passive,
    // set length of queue
    if (listen(sock, 64) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Run until cancelled
    while (int newSock = accept(sock, (struct sockaddr *) &echoclient, &clientlen))
    {
        // read a message from the client
        if ((received = read(newSock, buffer, 256)) < 0)
        {
            perror("Failed to receive message");
            exit(EXIT_FAILURE);
        }

        cerr << "Client connected: " << inet_ntoa(echoclient.sin_addr) << "\n";

        // write the message back to client
        if (write(newSock, buffer, received) != received)
        {
            perror("Mismatch in number of bytes");
            exit(EXIT_FAILURE);
        }
    }
}