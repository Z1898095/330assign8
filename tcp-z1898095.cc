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
#include <iostream>
#include <dirent.h>
#include <vector>

int port;
char* root_directory;

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        perror("Requires two computer arguments");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    root_directory = argv[2];

    // Attemp to open root dir
    printf("Attempting to open server root directory: %s\n", root_directory);
    DIR* p_rdir;
    p_rdir = opendir(root_directory);
    if (p_rdir == NULL)
    {
        perror("Server root directory does not exist\n");
        closedir(p_rdir);
        exit(EXIT_FAILURE);
    }
    closedir(p_rdir);
    
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
    if (bind(sock, (struct sockaddr *) &server_struct, sizeof(server_struct)) < 0)
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

    printf("Waiting for client connections.\n");
    // Accept client connections forever
    while (true)
    {
        socklen_t server_struct_len = sizeof(server_struct);
        int client_sock = accept(sock, (struct sockaddr *) &server_struct, &server_struct_len);
        // fork on new client connection
        int pid = fork(); // pid == 0 for child, child pid for parent
        if (pid == 0)
        {
            printf("Client connection created. socket = %d, pid = %d\n", client_sock, getpid());
            printf("Waiting for client message:\n");

            // read a message from the client
            char message[256];
            ssize_t received_len;
            if ((received_len = read(client_sock, message, 256)) < 0)
            {
                perror("Failed to receive message");
                shutdown(client_sock, SHUT_RD);
                exit(EXIT_FAILURE);
            }

            printf("Client message received:\n%s\n", message);

            // PARSE MESSAGE
            /**
             * GET /asdf                folder -> return contents of index.html from that folder if it exists
             *                            otherwise, return a list of files in the directory
             * GET /                    root directory
             * GET /asdf/               trailing '/' -> can be ignored
             * GET /asdf [other stuff]  -> ignore [other stuff] (truncate it)
             * GET /..                 .. -> crash with error
             * 
             */

            if (strlen(message) < 5)
            {
                // message is less than 5 bytes, no way it is ever valid
                printf("Request too short\n");
                shutdown(client_sock, SHUT_RD);
                exit(EXIT_FAILURE);
            }

            // check that prefix is equal to "GET /"
            char* msg_prefix = (char*) malloc(strlen(message));
            strncpy(msg_prefix, message, 5);
            if (strcmp("GET /", msg_prefix) != 0)
            {
                // message doesn't start with "GET /" so it is malformed
                perror("Request is malformed.");
                shutdown(client_sock, SHUT_RD);
                exit(EXIT_FAILURE);
            }

            // Concat root directory with message directory
            // e.g.
            // root_directory = "serverfiles"
            // message = "GET /dog"
            // directory -> "serverfiles/dog"
            char* directory = strdup(root_directory);
            strcat(directory, message + 4);

            // If we can open "index.html" or the directory itself (meaning it's a file), send it to client.
            char* index_dir = strdup(directory);
            strcat(index_dir, "index.html");
            printf("Attemping to open index.html file at %s\n", index_dir);
            FILE* fp = fopen(index_dir, "r");

            if (fp == NULL)
            {
                printf("Attemping to open file at %s\n", index_dir);
                fp = fopen(directory, "f");
            }

            if (fp != NULL)
            {
                perror("index.html or given file found\n");
                
                // send index.html to the client
                // read 1024 bytes at a time
                char buf[1024];
                size_t nread;

                while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0)
                {
                    if (ferror(fp))
                    {
                        perror("Failed to send index.html");
                        fclose(fp);
                        shutdown(client_sock, SHUT_RD);
                        exit(EXIT_FAILURE);
                    }

                    write(client_sock, buf, nread);
                }

                fclose(fp);
                shutdown(client_sock, SHUT_RD);
                exit(1);
            }
            printf("index.html or file not found\n");
            
            // try to open directory
            printf("Attempting to open directory: %s\n", directory);
            DIR* p_dir;
            p_dir = opendir(directory);
            if (p_dir == NULL)
            {
                perror("Directory does not exist\n");
                closedir(p_dir);
                shutdown(client_sock, SHUT_RD);
                exit(EXIT_FAILURE);
            }

            // loop through files in the directory and add the name of each file to a message
            printf ("Looking through files in directory:\n");
            char* return_message = (char*) malloc(sizeof(char) * 1);
            return_message[0] = '\0';
            struct dirent* p_direntry;

            while ((p_direntry = readdir(p_dir)) != NULL)
            {
                char* fname = p_direntry->d_name;
                if (fname[0] == '.')
                {
                    continue;
                }

                printf ("[%s]\n", fname);
                strcat(return_message, fname);
                strcat(return_message, " ");
            }

            closedir(p_dir);
            
            // remove trailing whitespace
            return_message[strlen(return_message) - 1] = '\0';

            //printf("MESSAGE TO SEND TO CLIENT: %s\n", return_message);

            // write the message back to client
            if (write(client_sock, return_message, sizeof(return_message)) != sizeof(return_message))
            {
                perror("Bytes lost when sending message");
                shutdown(client_sock, SHUT_RD);
                exit(EXIT_FAILURE);
            }

            shutdown(client_sock, SHUT_RD);
        }
        else if (pid == -1)
        {
            perror("fork failed");
            shutdown(client_sock, SHUT_RD);
            exit(EXIT_FAILURE);
        }
    }
}