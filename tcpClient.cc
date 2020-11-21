int sock;

// Create the TCP socket
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
}

// Construct the server sockaddr_in structure
memset(&echoserver, 0, sizeof(echoserver));      /* Clear struct */
echoserver.sin_family = AF_INET;                 /* Internet/IP */
echoserver.sin_addr.s_addr = inet_addr(argv[1]); /* IP address */
echoserver.sin_port = htons(atoi(argv[2]));      /* server port */


// connect to server
if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
{
    perror("cannot connect");
    exit(EXIT_FAILURE);
}

// Send the message to the server
echolen = strlen(argv[3]);
if (write(sock, argv[3], echolen) != echolen)
{
    perror("Mismatch in number of sent bytes");
    exit(EXIT_FAILURE);
}

// Receive the message back from the server
if ((received = read(sock, buffer, 256)) != echolen)
{
    perror("Mismatch in number of received bytes");
    exit(EXIT_FAILURE);
}

// Assure null-terminated string
buffer[received] = '\0';
cout << "Server (" << inet_ntoa(echoserver.sin_addr) << ") echoed: " << buffer << endl;