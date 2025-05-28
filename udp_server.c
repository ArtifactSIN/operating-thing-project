// udp_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1000
#define REQUEST_STRING "get route table"

void get_route_table(char *buffer)
{
    FILE *fp;
    char cmd_output[BUFFER_SIZE];
    int total_written = 0;

    memset(buffer, ' ', BUFFER_SIZE); // fill buffer with spaces
    buffer[BUFFER_SIZE - 1] = '\0';   // null-terminate for safety

    fp = popen("ip route show", "r");
    if (fp == NULL)
    {
        snprintf(buffer, BUFFER_SIZE, "Error getting the routing table !");
        return;
    }

    while (fgets(cmd_output, sizeof(cmd_output), fp) != NULL)
    {
        int len = strlen(cmd_output);
        if (total_written + len >= BUFFER_SIZE - 1)
            break;

        strncpy(buffer + total_written, cmd_output, len);
        total_written += len;
    }

    pclose(fp);

    // Ensure the string ends cleanly and fills the buffer
    if (total_written < BUFFER_SIZE)
        memset(buffer + total_written, ' ', BUFFER_SIZE - total_written - 1);
    buffer[BUFFER_SIZE - 1] = '\0'; // null-terminate
}

int main(int argc, char *argv[])
{
    int Sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client);

    // Defaults
    int Server_Port = 5005;
    char *Server_IP = "0.0.0.0";

    printf("You can run this program as follows: %s [server_PORT [server_IP]]\n", argv[0]);

    // 1. Create UDP socket
    if ((Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error. Socket not created.");
        exit(EXIT_FAILURE);
    }
    printf("Socket created. Descriptor: %d\n", Sock);

    if (argc > 1)
        Server_Port = atoi(argv[1]);
    if (argc > 2)
        Server_IP = argv[2];

    // 2. Bind socket to IP/port
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(Server_IP);
    server.sin_port = htons((u_short)Server_Port);

    if (bind(Sock, (const struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Error. Socket not bound.");
        close(Sock);
        exit(EXIT_FAILURE);
    }
    printf("Socket is bound to %s:%d\n", Server_IP, Server_Port);
    printf("UDP server listening...\n");

    // 3. Server loop
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        // Receive request
        int n = recvfrom(Sock, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client, &addr_len);
        if (n < 0)
        {
            perror("Receive failed");
            continue;
        }

        // Validate request
        if (strncmp(buffer, REQUEST_STRING, strlen(REQUEST_STRING)) == 0)
        {
            printf("The request is valid.\n");

            char response[BUFFER_SIZE];
            get_route_table(response);

            // Send response
            ssize_t sent = sendto(Sock, response, BUFFER_SIZE, 0,
            (struct sockaddr *)&client, addr_len);
            if (sent < 0) {
                perror("sendto() failed");
                close(Sock);
                exit(EXIT_FAILURE);
            }
            printf("Routing table sent to %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        }
        else
        {
            printf("The request is invalid.\n");
        }
    }

    close(Sock);
    return 0;
}
