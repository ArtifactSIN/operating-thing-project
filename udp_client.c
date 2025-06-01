#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_BUFFER_SIZE 1000         // kinda like the max message size we can send/get
#define GET_ROUTE_TABLE_CMD "get route table" // the command string to ask for the table

int main(int argc, char *argv[])
{
    int clientSocketFD;                         // socket thingy for the client
    struct sockaddr_in serverAddressInfo;       // server's address details
    struct sockaddr_in clientAddressInfo;       // my (client) address details
    char messageBuffer[MAX_BUFFER_SIZE];        // buffer to send/recv messages
    char userInputBuffer[MAX_BUFFER_SIZE];      // buffer for what I type

    // Default connection parameters if user doesn't provide them
    char *destinationServerIP = "127.0.0.1";    // server's IP if I don't give one
    int destinationServerPort = 5005;           // server's port if I don't give one
    char *localClientIP = "0.0.0.0";            // my IP, 0.0.0.0 means any interface
    int localClientPort = 6006;                 // my port

    printf("You can run this program as follows: %s [SERVER_PORT [SERVER_IP]]\n", argv[0]); // how to run this thing with args

    // 1. Make the client socket
    if ((clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error: Client socket not created."); // oops, socket didn't work
        exit(EXIT_FAILURE);
    }
    // printf("Client socket created successfully (FD: %d).\n", clientSocketFD);

    // 2. Tie the socket to my IP and port
    memset(&clientAddressInfo, 0, sizeof(struct sockaddr_in));      // zero out client address struct, good practice
    clientAddressInfo.sin_family = AF_INET;                         // using IPv4
    clientAddressInfo.sin_addr.s_addr = inet_addr(localClientIP);   // set my IP from the variable
    clientAddressInfo.sin_port = htons((u_short)localClientPort);   // set my port, htons for network byte order

    if (bind(clientSocketFD, (struct sockaddr *)&clientAddressInfo, sizeof(clientAddressInfo)) < 0)
    {
        perror("Error: Client socket not bound to local address."); // uh oh, bind failed
        close(clientSocketFD);
        exit(EXIT_FAILURE);
    }
    printf("Client socket bound to %s:%d\n", localClientIP, localClientPort);

    // Check if user gave server port/IP as args
    if (argc > 1)
    {
        destinationServerPort = atoi(argv[1]);  // get port from first arg
    }
    if (argc > 2)
    {
        destinationServerIP = argv[2];  // get IP from second arg
    }

    printf("Client will attempt to communicate with server at %s:%d\n", destinationServerIP, destinationServerPort);

    // 3. Setup server's address details for sending
    memset(&serverAddressInfo, 0, sizeof(serverAddressInfo));          // zero out server address struct
    serverAddressInfo.sin_family = AF_INET;                             // server is also IPv4
    serverAddressInfo.sin_port = htons(destinationServerPort);          // set server port from var (or default)
    if (inet_pton(AF_INET, destinationServerIP, &serverAddressInfo.sin_addr) <= 0) // convert server IP string to network format, check if valid
    {
        perror("Error: Invalid server IP address provided."); // bad server IP maybe?
        close(clientSocketFD);
        exit(EXIT_FAILURE);
    }

    printf("UDP client started. Listening on port %d.\n", localClientPort);
    printf("Type \"%s\" to request the routing table, or \"quit\" to exit.\n", GET_ROUTE_TABLE_CMD);

    // 4. Main loop: get command, send, receive response
    while (1)
    {
        printf("\nEnter command: ");
        if (fgets(userInputBuffer, sizeof(userInputBuffer), stdin) == NULL) // get my command from keyboard
        {
            // Handle EOF or input error
            if (feof(stdin))
            {
                printf("\nEOF detected. Exiting client.\n");
                break;
            }
            perror("Error reading input"); // problem reading what I typed
            continue;
        }

        // remove newline char from input, fgets keeps it
        userInputBuffer[strcspn(userInputBuffer, "\n")] = '\0';

        // check if I typed "quit"
        if (strcmp(userInputBuffer, "quit") == 0)
        {
            printf("Exit command received. Shutting down client.\n");
            break;
        }

        // check if it's the right command, otherwise complain
        if (strcmp(userInputBuffer, GET_ROUTE_TABLE_CMD) != 0)
        {
            printf("Invalid command. Please type \"%s\" or \"quit\".\n", GET_ROUTE_TABLE_CMD);
            continue;
        }

        // Prepare the request message buffer
        memset(messageBuffer, ' ', MAX_BUFFER_SIZE); // fill message with spaces (like the old code did, kinda weird but ok)
        strncpy(messageBuffer, GET_ROUTE_TABLE_CMD, strlen(GET_ROUTE_TABLE_CMD)); // copy command to message buffer
        messageBuffer[MAX_BUFFER_SIZE - 1] = '\0'; // make sure it's null-terminated, just in case

        // send the message to server
        // printf("Sending command: \"%.*s\"\n", (int)strlen(GET_ROUTE_TABLE_CMD), messageBuffer);
        if (sendto(clientSocketFD, messageBuffer, MAX_BUFFER_SIZE, 0, // Using MAX_BUFFER_SIZE
                   (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
        {
            perror("Error: Failed to send message to server."); // couldn't send, oh no
            break; // Exit loop on send error
        }

        // Receive the server's response
        socklen_t serverAddressLength = sizeof(serverAddressInfo);
        memset(messageBuffer, 0, MAX_BUFFER_SIZE); // clear buffer before getting new message from server

        // printf("Waiting for server response...\n");
        int bytesReceived = recvfrom(clientSocketFD, messageBuffer, MAX_BUFFER_SIZE, 0, // using MAX_BUFFER_SIZE
                                  (struct sockaddr *)&serverAddressInfo, &serverAddressLength); // get response from server
        if (bytesReceived < 0)
        {
            perror("Error: Failed to receive response from server."); // didn't get anything back or error
            break; // Exit loop on receive error
        }

        printf("\n--- Server Response (Routing Table) ---\n");
        // show what server sent, use MAX_BUFFER_SIZE to print potentially non-null-terminated data
        // (though using bytesReceived would be safer if server response is smaller and null-terminated)
        printf("%.*s", bytesReceived, messageBuffer); // Changed to bytesReceived for safer printing
        printf("\n--- End of Server Response ---\n");
    }

    // done with the socket, close it
    close(clientSocketFD);
    printf("Client shut down.\n");
    return 0; 
}

