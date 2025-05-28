#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_BUFFER_SIZE 1000         
#define GET_ROUTE_TABLE_CMD "get route table" 

int main(int argc, char *argv[])
{
    int clientSocketFD;                         
    struct sockaddr_in serverAddressInfo;       
    struct sockaddr_in clientAddressInfo;      
    char messageBuffer[MAX_BUFFER_SIZE];        
    char userInputBuffer[MAX_BUFFER_SIZE];      

    // Default connection parameters
    char *destinationServerIP = "127.0.0.1";   
    int destinationServerPort = 5005;          
    char *localClientIP = "0.0.0.0";            
    int localClientPort = 6006;                 

    printf("You can run this program as follows: %s [SERVER_PORT [SERVER_IP]]\n", argv[0]);

    // 1. Create UDP socket for the client
    if ((clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
    {
        perror("Error: Client socket not created.");
        exit(EXIT_FAILURE);
    }
    // printf("Client socket created successfully (FD: %d).\n", clientSocketFD);

    // 2. Bind client socket to a specific local IP and port
    memset(&clientAddressInfo, 0, sizeof(struct sockaddr_in));      
    clientAddressInfo.sin_family = AF_INET;                         
    clientAddressInfo.sin_addr.s_addr = inet_addr(localClientIP);    
    clientAddressInfo.sin_port = htons((u_short)localClientPort);    

    if (bind(clientSocketFD, (struct sockaddr *)&clientAddressInfo, sizeof(clientAddressInfo)) < 0) 
    {
        perror("Error: Client socket not bound to local address.");
        close(clientSocketFD); 
        exit(EXIT_FAILURE);
    }
    printf("Client socket bound to %s:%d\n", localClientIP, localClientPort); 

    // Process command-line arguments for server IP and Port
    if (argc > 1) 
    {
        destinationServerPort = atoi(argv[1]);  
    }
    if (argc > 2) 
    {
        destinationServerIP = argv[2];  
    }

    printf("Client will attempt to communicate with server at %s:%d\n", destinationServerIP, destinationServerPort); 

    // 3. Set up the destination server address information
    memset(&serverAddressInfo, 0, sizeof(serverAddressInfo));          
    serverAddressInfo.sin_family = AF_INET;                             
    serverAddressInfo.sin_port = htons(destinationServerPort);           
    if (inet_pton(AF_INET, destinationServerIP, &serverAddressInfo.sin_addr) <= 0) 
    {
        perror("Error: Invalid server IP address provided.");
        close(clientSocketFD); 
        exit(EXIT_FAILURE);
    }

    printf("UDP client started. Listening on port %d.\n", localClientPort); 
    printf("Type \"%s\" to request the routing table, or \"quit\" to exit.\n", GET_ROUTE_TABLE_CMD); 

    // 4. Main loop for sending commands and receiving responses
    while (1) 
    {
        printf("\nEnter command: ");
        if (fgets(userInputBuffer, sizeof(userInputBuffer), stdin) == NULL) 
        {
            // Handle EOF or input error
            if (feof(stdin))
            {
                printf("\nEOF detected. Exiting client.\n");
                break;
            }
            perror("Error reading input"); 
            continue;
        }

        // Remove newline character from fgets input
        userInputBuffer[strcspn(userInputBuffer, "\n")] = '\0'; 

        // Check for exit condition
        if (strcmp(userInputBuffer, "quit") == 0) 
        {
            printf("Exit command received. Shutting down client.\n");
            break;
        }

        // Check for valid command
        if (strcmp(userInputBuffer, GET_ROUTE_TABLE_CMD) != 0)  
        {
            printf("Invalid command. Please type \"%s\" or \"quit\".\n", GET_ROUTE_TABLE_CMD); 
            continue;
        }

        // Prepare the request message buffer
        memset(messageBuffer, ' ', MAX_BUFFER_SIZE); // Fill with spaces as per original logic 
        strncpy(messageBuffer, GET_ROUTE_TABLE_CMD, strlen(GET_ROUTE_TABLE_CMD)); 
        messageBuffer[MAX_BUFFER_SIZE - 1] = '\0'; // Ensure null-termination 

        // Send the request to the server
        // printf("Sending command: \"%.*s\"\n", (int)strlen(GET_ROUTE_TABLE_CMD), messageBuffer); 
        if (sendto(clientSocketFD, messageBuffer, MAX_BUFFER_SIZE, 0, // Using MAX_BUFFER_SIZE 
                   (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0) 
        {
            perror("Error: Failed to send message to server."); 
            break; // Exit loop on send error
        }

        // Receive the server's response
        socklen_t serverAddressLength = sizeof(serverAddressInfo); 
        memset(messageBuffer, 0, MAX_BUFFER_SIZE); // Clear buffer for receiving 
        
        // printf("Waiting for server response...\n"); 
        int bytesReceived = recvfrom(clientSocketFD, messageBuffer, MAX_BUFFER_SIZE, 0, // using MAX_BUFFER_SIZE 
                                  (struct sockaddr *)&serverAddressInfo, &serverAddressLength); 
        if (bytesReceived < 0) 
        {
            perror("Error: Failed to receive response from server."); 
            break; // Exit loop on receive error
        }
        
        printf("\n--- Server Response (Routing Table) ---\n");
        printf("%.*s", MAX_BUFFER_SIZE, messageBuffer); // Using MAX_BUFFER_SIZE
        printf("\n--- End of Server Response ---\n");
    }

    // Close the client socket
    close(clientSocketFD); 
    printf("Client shut down.\n"); 
    return 0;
}
