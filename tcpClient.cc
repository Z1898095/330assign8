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
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    int sock;

    // Create the TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_struct;                // structure for address of server
    // Construct the server sockaddr_in structure
    memset(&server_struct, 0, sizeof(server_struct));      /* Clear struct */
    server_struct.sin_family = AF_INET;                 /* Internet/IP */
    server_struct.sin_addr.s_addr = inet_addr(argv[1]); /* IP address */
    server_struct.sin_port = htons(atoi(argv[2]));      /* server port */


    // connect to server
    if (connect(sock, (struct sockaddr *) &server_struct, sizeof(server_struct)) < 0)
    {
        perror("cannot connect");
        exit(EXIT_FAILURE);
    }

    // Send the message to the server
    char* message = argv[3];
    if (write(sock, message, sizeof(message)) != sizeof(message))
    {
        perror("Mismatch in number of sent bytes");
        exit(EXIT_FAILURE);
    }

    // Receive the message back from the server
    char buffer[256];
    size_t received;
    if ((received = read(sock, buffer, 256)) != sizeof(message))
    {
        perror("Mismatch in number of received bytes");
        exit(EXIT_FAILURE);
    }

    // Assure null-terminated string
    buffer[received] = '\0';
    std::cout << "Server (" << inet_ntoa(server_struct.sin_addr) << ") echoed: " << buffer << endl;
}