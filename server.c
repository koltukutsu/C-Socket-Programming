#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3000
#define MAX_BUFFER_SIZE 1024

// messages
typedef struct message{
    char correspondentId[10];
    char message[255];
}Message;
// correspondent contact
typedef struct contact {
    char contactUserId[10];
    char phoneNumber[13];
    char name[15];
    char surname[15];
}Contact;

// user 
typedef struct user {
    char userId[10];
    Contact contactsList[100];
    Message messagesList[255];
} User;

void handleClient(int client_socket, User *userList);
void sendContacts(char *userId, User *userList);
void addUser();
void deleteUser();
void deleteUser();
void takeMessages();
void checkMessage();
int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    User userList[100];
    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port: %d\n", PORT);

    while (1) {
        // Accept a connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
            perror("Error accepting connection");
            continue;
        }

        printf("Connection accepted from %s\n", inet_ntoa(client_address.sin_addr));

        // Handle the client's request
        handleClient(client_socket, userList);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}


void handleClient(int client_socket, User *userList) {
    char buffer[MAX_BUFFER_SIZE];

    // Receive command from the client
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes < 0) {
        perror("Error receiving data from client");
        close(client_socket);
        return;
    }

    buffer[received_bytes] = '\0'; // Null-terminate the received data

    if(strncmp(buffer, "takeContacts", 12) == 0) {
        
        char userId[4];
        int pos = 13;
         strncpy(userId, buffer + (pos - 1), 3);
        // take the user id from the buffer
        sendContacts(userId, userList);
    }

    if (strcmp(buffer, "send") == 0) {
        // If the command is "send," send a message to the client
        const char *message = "Hello from the server!";
        send(client_socket, message, strlen(message), 0);
    } else if (strcmp(buffer, "take") == 0) {
        // If the command is "take," receive a message and save it to a file
        received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (received_bytes < 0) {
            perror("Error receiving data from client");
            close(client_socket);
            return;
        }
        buffer[received_bytes] = '\0'; // Null-terminate the received data

        // Save the message to a file named "output.txt"
        FILE *file = fopen("output.txt", "w");
        if (file == NULL) {
            perror("Error opening file");
            close(client_socket);
            return;
        }
        fprintf(file, "%s", buffer);
        fclose(file);
    }

    // Close the connection
    close(client_socket);
}

void sendContacts(char userId[4], User *userList){
    char delimiterContact = ':';
    char delimeterFields = '_';
    int counter = 0;
    while(strcmp(userList[counter].userId, userId) != 0 && counter < 100) {
        counter ++;
    }

    if(strcmp(userList[counter].userId, userId) == 0) {
        // write to buffer

        // send the buffer
    }

    return;
    

}

void addUser(){

}
void deleteUser(){

}

void takeMessages(){

}

void checkMessage(){}