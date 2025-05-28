#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1000
#define REQUEST_STRING "get route table"

int main(int argc, char *argv[])
{
    int Sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // Defaults
    char *Server_IP = "127.0.0.1";
    int Server_Port = 5005;
    char *Client_IP = "0.0.0.0";
    int Client_Port = 6006;

    printf("You can run this program as follows: %s [server_PORT [server_IP]]\n", argv[0]);

    // 1. Create UDP socket
    if ((Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error. Socket not created.");
        exit(EXIT_FAILURE);
    }

    // 2. Bind client socket to a local port
    memset(&client, 0, sizeof(struct sockaddr_in));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(Client_IP);
    client.sin_port = htons((u_short)Client_Port);

    if (bind(Sock, (struct sockaddr *)&client, sizeof(client)) < 0)
    {
        perror("Error. Socket not bound.");
        close(Sock);
        exit(EXIT_FAILURE);
    }
    printf("Client socket bound to port %d\n", Client_Port);

    if (argc > 1)
        Server_Port = atoi(argv[1]);
    if (argc > 2)
        Server_IP = argv[2];

    printf("Sock will communicate with %s:%d\n", Server_IP, Server_Port);

    // 3. Set up server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(Server_Port);
    if (inet_pton(AF_INET, Server_IP, &server.sin_addr) <= 0)
    {
        perror("Invalid server IP address.");
        close(Sock);
        exit(EXIT_FAILURE);
    }

    printf("UDP client started on port %d. Type \"get route table\" or \"quit\".\n", Client_Port);

    // 4. Loop for user interaction
    while (1)
    {
        printf("\nEnter command: ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Input error.\n");
            continue;
        }

        // Remove newline if present
        input[strcspn(input, "\n")] = '\0';

        // Exit condition
        if (strcmp(input, "quit") == 0)
            break;

        if (strcmp(input, REQUEST_STRING) != 0)
        {
            printf("Invalid command. Try again.\n");
            continue;
        }

        // Prepare request buffer
        memset(buffer, ' ', BUFFER_SIZE);
        strncpy(buffer, REQUEST_STRING, strlen(REQUEST_STRING));
        buffer[BUFFER_SIZE - 1] = '\0';

        // Send request
        if (sendto(Sock, buffer, BUFFER_SIZE, 0,
                   (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("Error sending message");
            break;
        }

        // Receive response
        socklen_t addr_len = sizeof(server);
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(Sock, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&server, &addr_len);
        if (n < 0)
        {
            perror("Error receiving response");
            break;
        }

        // Print the routing table
        printf("\nReceived routing table:\n%.*s\n", BUFFER_SIZE, buffer);
    }

    close(Sock);
    printf("Client exited.\n");
    return 0;
}
