// Network
#include <arpa/inet.h>

// Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Threading
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8004
#define MAX_BUFFER_SIZE 1024

typedef struct {
    char id[10];
    char name[30];
    char surname[15];
    char phone[14];
} UserInfo;


typedef struct client_t {
    char userId[4];
    int client_socket;
} client_t;

int showMenu();

// void sendUserId(int client_socket, char userId[4]);
void *receive_messages(void *arg);

void userChoiceOperations(int choice, int client_socket, char userId[4]);

//// MENU Operations
void getContacts(char userId[4], int client_socket);

void addUser(char userId[4], int client_socket);

void deleteUser(char userId[4], int client_socket);

void sendMessages(char userId[4], int client_socket);

void checkMessages(char userId[4], int client_socket);

//// UTILS
void promptForUserInfo(UserInfo *userInfo);
void formatUserId(char *input, char *userId);

int main(int argc, char *argv[]) {
    int client_socket;
    int choice;
    struct sockaddr_in server_address;
    char userId[4];
//  pthread_t recv_thread;

    // server id control
    if (argc != 2) {
        printf("CLIENT - Give a user id only as an argument.\n");
        printf("CLIENT - Program is closing...");
        return 32;
    } else {
        printf("%s\n", argv[1]);
        formatUserId(argv[1], userId);
        // strcpy(userId, argv[1]);
        printf("CLIENT %s - Assigned User ID: %s\n", userId, userId);
    }
    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    } else {
        printf("CLIENT %s - Connected to the server.\n", userId);
    }

    // Set up the server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_aton(SERVER_IP, &server_address.sin_addr);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *) &server_address,
                sizeof(server_address)) == -1) {
        perror("Error connecting to the server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Create a thread for receiving messages from the server
//  if (pthread_create(&recv_thread, NULL, receive_messages, &client_socket) != 0)
//  {
//    perror("Thread creation failed");
//    exit(EXIT_FAILURE);
//  }

    do {
        choice = showMenu();
        userChoiceOperations(choice, client_socket, userId);
    } while (choice != 6);
    // Close the client socket
    // close the thread for the 'always listener'
//  pthread_cancel(recv_thread);
    close(client_socket);
    pthread_exit(NULL);

    return 0;
}

int showMenu() {
    int choice;
    bool _stop;
    _stop = false;
    do {
        printf("\n\n<--- MENU ---> \n");
        printf("1. List Contacts\n");
        printf("2. Add User\n");
        printf("3. Delete user\n");
        printf("4. Send Messages\n");
        printf("5. Check Messages\n");
        printf("6. Close the client\n");
        printf("Choice: ");
        scanf("%d", &choice);

        if (choice > 6) {
            if (choice <= 0)
                _stop = true;
        }
    } while (_stop);
    return choice;
}
// void sendUserId(int client_socket, char userId[4]) {
//   printf("CLIENT %s - Sending User Id: %s\n", userId, userId);
//   send(client_socket, userId, strlen(userId), 0);

void *receive_messages(void *arg) {
//  int client_socket = *(int *)arg;
//  char buffer[MAX_BUFFER_SIZE];
//  while (1)
//  {
//    memset(buffer, 0, sizeof(buffer));
//    if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0)
//    {
//      perror("Server disconnected.");
//      exit(EXIT_FAILURE);
//    }
//    printf("Server: %s\n", buffer);
//  }
}

//   // server response
//   char buffer[MAX_BUFFER_SIZE];
//   ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
//   if (received_bytes > 0) {
//     buffer[received_bytes] = '\0'; // Null-terminate the received data
//     printf("Server response: %s\n", buffer);
//   } else {
//     perror("Error receiving data from server");
//   }
// }
void userChoiceOperations(int choice, int client_socket, char userId[4]) {

    switch (choice) {
        case 1:
            printf("\tYour Contacts:\n");
            // pthread_create(&tid, NULL, getContacts, &client_info);
            getContacts(userId, client_socket);
            break;
        case 2:
            printf("\tAdding User:\n");
            // pthread_create(&tid, NULL, addUser, &client_info);
            addUser(userId, client_socket);
            break;
        case 3:
            printf("\tDeleting User:\n");
            // pthread_create(&tid, NULL, deleteUser, &client_info);
            deleteUser(userId, client_socket);
            break;
        case 4:
            printf("\tSending Messages:\n");
            // pthread_create(&tid, NULL, sendMessages, &client_info);
            sendMessages(userId, client_socket);
            break;
        case 5:
            // pthread_create(&tid, NULL, checkMessages, &client_info);
            printf("\tChecking Messages:\n");
            checkMessages(userId, client_socket);
            break;
        case 6:
            printf("\tClosing the client now...\n");
            break;
        default:
            break;
    }
}

//// MENU
// 1. get contacts
void getContacts(char userId[4], int client_socket) {
    char command[MAX_BUFFER_SIZE];
    strcpy(command, userId);
    strcat(command, ":1");
    puts("\nCLIENT - get contacts: ");
    puts(command);
    send(client_socket, command, strlen(command), 0);

    // Server response
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", buffer);

        // Check if the response is a user not found message
        if (strstr(buffer, "User ") && strstr(buffer, " not found")) {
            printf("Error: %s\n", buffer);
        } else {
            // Print header
            printf("\n%-10s %-13s %-15s %-15s\n", "User ID", "Phone Number", "Name", "Surname");

            // Process the response and print each contact
            char *token = strtok(buffer, "_");
            while (token != NULL) {
                char contactId[10], phoneNumber[14], name[30], surname[15];
                if (sscanf(token, "%9[^:]:%13[^:]:%29[^:]:%14[^:]", contactId, phoneNumber, name, surname) == 4) {
                    printf("%-10s %-13s %-15s %-15s\n", contactId, phoneNumber, name, surname);
                } else if (strstr(token, "There is no")) {
                    printf("No Contact: %s, you can always add your contacts!!!\n", token);

                } else {
                    printf("Invalid contact format: %s\n", token);
                }
                token = strtok(NULL, "_");
            }
        }
    } else {
        perror("Error receiving data from server");
    }
}


// 2. add user
void addUser(char userId[4], int client_socket) {
    UserInfo userInfo;
    promptForUserInfo(&userInfo);

    char message[MAX_BUFFER_SIZE];
    snprintf(message, MAX_BUFFER_SIZE, "%s:2:%s_%s_%s_%s", userId, userInfo.id, userInfo.phone, userInfo.name,
             userInfo.surname);

    printf("Client - ADD User: %s\n", message);
    if (send(client_socket, message, strlen(message), 0) < 0) {
        perror("Error sending data to server");
        return;
    }
    printf("Client - Now waiting for the server response...\n");
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", buffer);
    } else {
        perror("Error receiving data from server");
    }
}

// 3. delete user
void deleteUser(char userId[4], int client_socket) {

    // client function
    char command[MAX_BUFFER_SIZE];
    strcpy(command, userId);
    strcat(command, ":3:");
    // delete user things
    send(client_socket, command, strlen(command), 0);

    // server response
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", buffer);
    } else {
        perror("Error receiving data from server");
    }
}

// 4. send messages
void sendMessages(char userId[4], int client_socket) {
    char recipientId[4];

    printf("Enter recipient's User ID: ");
    scanf("%3s", recipientId);
    getchar();  // Clear newline character from the buffer

    //// FIRST
    // First part: Send the client userID and mode
    char initialCommand[MAX_BUFFER_SIZE];
    snprintf(initialCommand, sizeof(initialCommand), "%s:4:%s", userId, recipientId);
    send(client_socket, initialCommand, strlen(initialCommand), 0);
    //First Part Server response
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);

    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
//        printf("Server response: %s\n", buffer);
        if(strstr(buffer, "Recipient not found")){
            printf("\033[31m\tError: %s\n\033[0m", buffer);
            return;
        } else {
            printf("\033[32m\tFound: %s\n\033[0m", buffer);
        }
    } else {
        perror("Client - Error receiving data from server");
        return;
    }

    //// SECOND
    // third part: send the message
    char message[255];
    printf("Enter your message: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = 0;  // Remove newline character
    // third part: Send the message to the recipient
    char fullMessage[MAX_BUFFER_SIZE];
    snprintf(fullMessage, sizeof(fullMessage), "%s",message);
    send(client_socket, fullMessage, strlen(fullMessage), 0);

    char bufferFinal[MAX_BUFFER_SIZE];
    ssize_t received_bytes_final = recv(client_socket, bufferFinal, sizeof(bufferFinal), 0);
    if (received_bytes_final > 0) {
        bufferFinal[received_bytes_final] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", bufferFinal);
    } else {
        perror("Client - Error receiving data from server");
        return;
    }
    // finish the process
    return;
}


// 5. check messages
void checkMessages(char userId[4], int client_socket) {
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s:5", userId); // 5 is the code for checkMessages
    send(client_socket, command, strlen(command), 0);

    // Receive the list of senders from the server
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    buffer[received_bytes] = '\0';
    if(received_bytes > 0)
    {
//        printf("Server response: %s\n", buffer);
        if(strstr(buffer, "No message")){
            printf("There are no messages yet!\n");
            return;
        } else {
            printf("User has messages, continuing...\n");
            printf("You have messages from the following users:\n%s", buffer);

            printf("Whose messages do you want to read? Enter User ID: ");
            char senderId[4];
            scanf("%3s", senderId);

            // Now, request the specific messages from the server
            // The server would need to handle this additional request
            snprintf(command, sizeof(command), "%s:5:%s", userId, senderId); // Additional info for specific user's messages
            send(client_socket, command, strlen(command), 0);

            // Receive the messages
            received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
            buffer[received_bytes] = '\0';

            // Display the messages
            printf("Messages from User %s:\n%s", senderId, buffer);
        }
    } else {
        perror("Error receiving data from server");
        return;
    }

}


//// Utils
void promptForUserInfo(UserInfo *userInfo) {
    printf("\nEnter ID, Mobile Phone, Name, Surname, in order:\n");

    printf("ID: ");
    scanf("%9s", userInfo->id);
    // Clear the input buffer
    while (getchar() != '\n');

    printf("Phone: ");
    scanf("%13s", userInfo->phone);
    // Clear the input buffer
    while (getchar() != '\n');

    printf("Name: ");
    // Read up to 29 characters until a newline is encountered
    scanf("%29[^\n]", userInfo->name);
    // Clear the input buffer
    while (getchar() != '\n');

    printf("Surname: ");
    // Read up to 14 characters until a newline is encountered
    scanf("%14[^\n]", userInfo->surname);
}
void formatUserId(char *input, char *userId) {
    if (strlen(input) == 2) {
        userId[0] = '0';
        userId[1] = input[0];
        userId[2] = input[1];
    } else if (strlen(input) == 1) {
        userId[0] = '0';
        userId[1] = '0';
        userId[2] = input[0];
    } else {
        userId[0] = input[0];
        userId[1] = input[1];
        userId[2] = input[2];
    }
    userId[3] = '\0';  // Ensure string is null-terminated
}
