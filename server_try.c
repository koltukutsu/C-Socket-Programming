#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8082
#define MAX_CLIENTS 5

char options[][50] = {"Option 1: Hello from server!", "Option 2: Goodbye from server!"};

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[1024];
    int option;

    while (1) {
        option = rand() % 2;  // Randomly choose an option

        sprintf(buffer, "%s\n", options[option]);

        send(client_socket, buffer, strlen(buffer), 0);
        
        // Check if the client wants to continue
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0 || buffer[0] != 'a') {
            break;  // Exit the loop if the client doesn't want to continue
        }
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    pthread_t tid;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error in socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in binding");
        exit(1);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) == 0) {
        printf("Server listening...\n");
    } else {
        perror("Error in listening");
        exit(1);
    }

    while (1) {
        // Accept a client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);

        if (client_socket < 0) {
            perror("Error in accepting");
            exit(1);
        }

        // Create a thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Error in creating thread");
            exit(1);
        }
        
        pthread_detach(tid); // Detach the thread to allow it to clean up when it's done
    }

    close(server_socket);
    return 0;
}
