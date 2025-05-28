#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_BUFFER_SIZE 1000             
#define EXPECTED_REQUEST_CMD "get route table" 

// Function to get the routing table and format it into the outputBuffer
void fetchAndFormatRouteTable(char *outputBuffer) 
{
    FILE *commandPipe;                      
    char commandLineOutput[MAX_BUFFER_SIZE];     
    int bytesWrittenToBuffer = 0;            

    // Initialize buffer: fill with spaces, then null-terminate the end
    memset(outputBuffer, ' ', MAX_BUFFER_SIZE);  
    outputBuffer[MAX_BUFFER_SIZE - 1] = '\0';    

    // Open a pipe to the 'ip route show' command
    // printf("Executing 'ip route show' command to fetch routing table...\n");
    commandPipe = popen("ip route show", "r"); 
    if (commandPipe == NULL) 
    {
        perror("Error: Failed to run 'ip route show' command via popen"); 
        // Prepare an error message in the output buffer
        snprintf(outputBuffer, MAX_BUFFER_SIZE, "Error: Could not retrieve routing table from system!"); 
        // Ensure the rest of the buffer is padded with spaces if snprintf wrote less than MAX_BUFFER_SIZE
        int errMsgLen = strlen(outputBuffer);
        if (errMsgLen < MAX_BUFFER_SIZE - 1)
        {
             memset(outputBuffer + errMsgLen, ' ', MAX_BUFFER_SIZE - errMsgLen - 1);
        }
        outputBuffer[MAX_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
        return;
    }

    // Read the command output line by line
    while (fgets(commandLineOutput, sizeof(commandLineOutput), commandPipe) != NULL) 
    {
        int currentLineLength = strlen(commandLineOutput); 
        // Check if there's enough space in the output buffer (leaving room for final null terminator)
        if (bytesWrittenToBuffer + currentLineLength >= MAX_BUFFER_SIZE - 1) // 
        {
            // printf("Warning: Routing table output may be truncated as buffer is nearly full.\n"); 
            break; // Stop if buffer is effectively full
        }

        // Copy the current line of command output to the output buffer
        strncpy(outputBuffer + bytesWrittenToBuffer, commandLineOutput, currentLineLength); // 
        bytesWrittenToBuffer += currentLineLength; // 
    }

    pclose(commandPipe); // 
    // printf("Finished reading from 'ip route show' command pipe.\n"); 

    // The buffer was pre-filled with spaces.
    // Any part not overwritten by strncpy will remain spaces.
    // The final character is already set to '\0'.
    // Original logic to explicitly fill remaining with spaces after reading:
    // if (bytesWrittenToBuffer < MAX_BUFFER_SIZE -1) // if any space left before the last char
    //    memset(outputBuffer + bytesWrittenToBuffer, ' ', MAX_BUFFER_SIZE - bytesWrittenToBuffer - 1);
    // This is largely handled by the initial memset, but being explicit ensures no intermediate nulls from command output become end of data.
    // However, the client expects a full MAX_BUFFER_SIZE block. The initial fill should suffice.
}

int main(int argc, char *argv[])
{
    int serverSocketFD;                               // Renamed from Sock 
    struct sockaddr_in serverBindingInfo;             // Renamed from server 
    struct sockaddr_in incomingClientAddress;         // Renamed from client 
    char receiveBuffer[MAX_BUFFER_SIZE];              // Renamed from buffer 
    socklen_t clientAddressLength = sizeof(incomingClientAddress); // Renamed from addr_len 

    // Default server parameters
    int listeningPort = 5005;                         // Renamed from Server_Port 
    char *listeningIPAddress = "0.0.0.0";             // Renamed from Server_IP 

    printf("You can run this server as follows: %s [SERVER_PORT [SERVER_IP]]\n", argv[0]); // 

    // 1. Create UDP socket for the server
    if ((serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) // 
    {
        perror("Error: Server socket not created."); // 
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully. File Descriptor: %d\n", serverSocketFD); // 

    // Process command-line arguments for server IP and Port
    if (argc > 1) // 
    {
        listeningPort = atoi(argv[1]); // Using renamed variable 
    }
    if (argc > 2) // 
    {
        listeningIPAddress = argv[2]; // Using renamed variable 
    }

    // 2. Bind the server socket to a specific IP address and port
    memset(&serverBindingInfo, 0, sizeof(struct sockaddr_in));           
    serverBindingInfo.sin_family = AF_INET;                             
    serverBindingInfo.sin_addr.s_addr = inet_addr(listeningIPAddress);  
    serverBindingInfo.sin_port = htons((u_short)listeningPort);         

    if (bind(serverSocketFD, (const struct sockaddr *)&serverBindingInfo, sizeof(serverBindingInfo)) < 0) 
    {
        perror("Error: Server socket not bound to address."); 
        close(serverSocketFD); 
        exit(EXIT_FAILURE);
    }
    printf("Server socket is bound to %s:%d\n", listeningIPAddress, listeningPort); 
    printf("UDP server is up and listening for incoming requests...\n"); 

    // 3. Main server loop to handle incoming client requests
    while (1) // 
    {
        memset(receiveBuffer, 0, MAX_BUFFER_SIZE); // Clear buffer for new message 
        // printf("\nWaiting to receive message from a client...\n"); 

        // Receive a request from a client
        int bytesReceived = recvfrom(serverSocketFD, receiveBuffer, MAX_BUFFER_SIZE, 0, 
                                  (struct sockaddr *)&incomingClientAddress, &clientAddressLength);  
        if (bytesReceived < 0) 
        {
            perror("Error: Failed to receive message from client."); 
            continue; // Continue to next iteration rather than exiting server
        }
        
        printf("\nReceived a request from client %s:%d (%d bytes)\n",
               inet_ntoa(incomingClientAddress.sin_addr), 
               ntohs(incomingClientAddress.sin_port), 
               bytesReceived);
        // printf("Raw received data (first %d bytes): \"%.*s\"\n", bytesReceived, bytesReceived, receiveBuffer); 

        // Validate the received request (client sends fixed size buffer, command at the beginning)
        if (strncmp(receiveBuffer, EXPECTED_REQUEST_CMD, strlen(EXPECTED_REQUEST_CMD)) == 0) 
        {
            printf("Client request is valid: \"%s\"\n", EXPECTED_REQUEST_CMD); 

            char routeTableResponse[MAX_BUFFER_SIZE];       
            fetchAndFormatRouteTable(routeTableResponse);    

            // Send the routing table back to the client
            // printf("Sending routing table response to client (%d bytes)...\n", MAX_BUFFER_SIZE); 
            ssize_t bytesSent = sendto(serverSocketFD, routeTableResponse, MAX_BUFFER_SIZE, 0, // Using MAX_BUFFER_SIZE
                                  (struct sockaddr *)&incomingClientAddress, clientAddressLength); // 
            if (bytesSent < 0) // 
            {
                perror("Error: Failed to send response to client."); // 
                // Original code exits on this failure.
                close(serverSocketFD); // 
                exit(EXIT_FAILURE);    // 
            }
            else if (bytesSent != MAX_BUFFER_SIZE)
            {
                printf("Warning: sendto may not have sent the entire buffer. Sent %zd of %d bytes.\n", bytesSent, MAX_BUFFER_SIZE);
            }
            printf("Routing table successfully sent to client %s:%d\n",
                   inet_ntoa(incomingClientAddress.sin_addr), ntohs(incomingClientAddress.sin_port)); // 
        }
        else // 
        {
            printf("Client request is invalid or not recognized.\n"); // 
            // printf("Received data did not match expected command: \"%.*s\"\n", (int)strlen(EXPECTED_REQUEST_CMD), receiveBuffer); 
        }
    }

    // Close the server socket (this part is unreachable in the current while(1) loop)
    printf("Server shutting down (this message is typically unreachable)...\n");
    close(serverSocketFD); // 
    return 0; // 
}
