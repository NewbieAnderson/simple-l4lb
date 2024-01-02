#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4895
#define MAX_BUFFER_SIZE 1024

int main(void)
{
    int server_socket;
    int client_socket;
    int flag_reuse = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &flag_reuse, sizeof(flag_reuse)) == -1) {
        perror("Error setting socket options");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding address");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size)) == -1) {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
    }

    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    while (1) {
        printf("receiving...\n");
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Connection closed by the client\n");
            } else {
                perror("Error receiving data ");
            }
            break;
        }

        printf("received message: %.*s\n", (int)bytes_received, buffer);

        /*
        ssize_t bytes_sent = send(client_socket, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("Error sending data ");
            break;
        }
        */
    }

    close(client_socket);
    close(server_socket);
    return 0;
}
