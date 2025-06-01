#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1000
#define EXPECTED_REQUEST_CMD "get route table"

// This function gets and formats the routing table
void fetchAndFormatRouteTable(char *outputBuffer) // the parameter is a pointer where the routing table will be stored
{
    FILE *commandPipe;
    char commandLineOutput[MAX_BUFFER_SIZE];
    int bytesWrittenToBuffer = 0;

    // Initializing the output buffer with spaces and null-terminate
    memset(outputBuffer, ' ', MAX_BUFFER_SIZE);
    outputBuffer[MAX_BUFFER_SIZE - 1] = '\0';

    // Open a pipe to command
    printf("Fetching routing table...\n");
    commandPipe = popen("ip route show", "r"); // run the command and capture the output
    if (commandPipe == NULL)
    {
        perror("Error: Failed to run 'ip route show' command!"); // displayed in terminal

        snprintf(outputBuffer, MAX_BUFFER_SIZE, "Error: Could not retrieve routing table!"); // send to the client
        // In case of the output is less than MAX_BUFFER_SIZE, fill the rest of the buffer with space
        int errMsgLen = strlen(outputBuffer);
        if (errMsgLen < MAX_BUFFER_SIZE - 1)
        {
            memset(outputBuffer + errMsgLen, ' ', MAX_BUFFER_SIZE - errMsgLen - 1);
        }
        outputBuffer[MAX_BUFFER_SIZE - 1] = '\0';
        return;
    }

    // Read the output and append to buffer
    while (fgets(commandLineOutput, sizeof(commandLineOutput), commandPipe) != NULL)
    {
        int currentLineLength = strlen(commandLineOutput);

        if (bytesWrittenToBuffer + currentLineLength >= MAX_BUFFER_SIZE - 1)
        {
            break;
        }

        strncpy(outputBuffer + bytesWrittenToBuffer, commandLineOutput, currentLineLength);
        bytesWrittenToBuffer += currentLineLength;
    }

    // Fill the rest of the buffer with space
    if (bytesWrittenToBuffer < MAX_BUFFER_SIZE - 1)
        memset(outputBuffer + bytesWrittenToBuffer, ' ', MAX_BUFFER_SIZE - bytesWrittenToBuffer - 1);
    outputBuffer[MAX_BUFFER_SIZE - 1] = '\0';

    pclose(commandPipe);
}

int main(int argc, char *argv[])
{
    int serverSocketFD;                       // file descriptor
    struct sockaddr_in serverBindingInfo;     // structure containing the address and port to be assigned
    struct sockaddr_in incomingClientAddress; // address information for client
    char receiveBuffer[MAX_BUFFER_SIZE];
    socklen_t clientAddressLength = sizeof(incomingClientAddress);

    // Default server parameters
    int listeningPort = 5005;
    char *listeningIPAddress = "0.0.0.0";

    printf("You can run this server as follows: %s [SERVER_PORT [SERVER_IP]]\n", argv[0]);

    // 1. Create UDP socket for the server
    if ((serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error: Server socket not created.");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully. File Descriptor: %d\n", serverSocketFD);

    // Getting the server IP and port information from the command-line (optional)
    if (argc > 1) //
    {
        listeningPort = atoi(argv[1]);
    }
    if (argc > 2) //
    {
        listeningIPAddress = argv[2];
    }

    // 2. Bind the server socket to IP address and port
    memset(&serverBindingInfo, 0, sizeof(struct sockaddr_in));
    serverBindingInfo.sin_family = AF_INET;
    serverBindingInfo.sin_addr.s_addr = inet_addr(listeningIPAddress);
    serverBindingInfo.sin_port = htons((u_short)listeningPort);

    if (bind(serverSocketFD, (const struct sockaddr *)&serverBindingInfo, sizeof(serverBindingInfo)) < 0)
    {
        perror("Error: Server socket not bound.");
        close(serverSocketFD);
        exit(EXIT_FAILURE);
    }
    printf("Server socket is bound to %s:%d\n", listeningIPAddress, listeningPort);
    printf("UDP server is up and listening...\n");

    // 3. Server Loop
    while (1)
    {
        memset(receiveBuffer, 0, MAX_BUFFER_SIZE); // Clear the buffer

        // Receive request
        int bytesReceived = recvfrom(serverSocketFD, receiveBuffer, MAX_BUFFER_SIZE, 0,
                                     (struct sockaddr *)&incomingClientAddress, &clientAddressLength);
        if (bytesReceived < 0)
        {
            perror("Error: Failed to receive message.");
            continue;
        }

        printf("\nReceived a request from client %s:%d (%d bytes)\n",
               inet_ntoa(incomingClientAddress.sin_addr),
               ntohs(incomingClientAddress.sin_port),
               bytesReceived);

        // Validate the received request
        if (strncmp(receiveBuffer, EXPECTED_REQUEST_CMD, strlen(EXPECTED_REQUEST_CMD)) == 0)
        {
            printf("The request is valid: \"%s\"\n", EXPECTED_REQUEST_CMD);

            char routeTableResponse[MAX_BUFFER_SIZE];
            fetchAndFormatRouteTable(routeTableResponse);

            // Send the routing table
            ssize_t bytesSent = sendto(serverSocketFD, routeTableResponse, MAX_BUFFER_SIZE, 0,
                                       (struct sockaddr *)&incomingClientAddress, clientAddressLength);
            if (bytesSent < 0)
            {
                perror("Error: Failed to send response to client.");
                close(serverSocketFD);
                exit(EXIT_FAILURE);
            }
            else if (bytesSent != MAX_BUFFER_SIZE)
            {
                printf("Warning: sendto have not sent the entire buffer. Sent %zd of %d bytes.\n", bytesSent, MAX_BUFFER_SIZE);
            }
            printf("Routing table successfully sent to client %s:%d\n",
                   inet_ntoa(incomingClientAddress.sin_addr), ntohs(incomingClientAddress.sin_port));
        }
        else
        {
            printf("Client request is invalid.\n");
        }
    }

    // Close the socket
    printf("Server shutting down...\n");
    close(serverSocketFD);
    return 0;
}
