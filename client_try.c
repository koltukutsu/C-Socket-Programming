#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define MAX_BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];
    char request[2];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error in socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in connection");
        exit(1);
    }

    while (1) {
        printf("Press 'a' to send a request to the server (or 'q' to quit): ");
        fgets(request, sizeof(request), stdin);

        if (request[0] == 'q') {
            break;  // Exit the loop and close the client
        } else if (request[0] == 'a') {
            // Send the request to the server
            send(client_socket, request, sizeof(request), 0);

            // Receive and print the server's response
            recv(client_socket, buffer, sizeof(buffer), 0);
            printf("Server says: %s\n", buffer);
        } else {
            printf("Invalid input. Press 'a' to send a request or 'q' to quit.\n");
        }
    }

    // Close the socket
    close(client_socket);

    return 0;
}
