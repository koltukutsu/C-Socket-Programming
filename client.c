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
#define PORT 4003
#define MAX_BUFFER_SIZE 1024

typedef struct {
    char id[10];
    char name[30];
    char surname[15];
    char phone[14];
} UserInfo;


typedef struct client_t {
    char userId[4];
    int clientSocket;
} clientType;

int showMenu();

// void sendUserId(int clientSocket, char userId[4]);
void userChoiceOperations(int choice, int clientSocket, char userId[4]);

//// MENU Operations
void getContacts(char userId[4], int clientSocket);

void addUser(char userId[4], int clientSocket);

void deleteUser(char userId[4], int clientSocket);

void sendMessages(char userId[4], int clientSocket);

void checkMessages(char userId[4], int clientSocket);

//// UTILS
bool promptForUserInfo(char userId[4], UserInfo *userInfo);

void formatUserId(char *input, char *userId);

char *formatUserIdOneInput(const char *input);

int main(int argc, char *argv[]) {
    int clientSocket;
    int choice;
    struct sockaddr_in serverAddress;
    char userId[4];
//  pthread_t recv_thread;

    // server id control
    if (argc != 2) {
        printf("\033[31m\tCLIENT - Give a user id only as an argument.\n\033[0m");
        printf("\033[31m\tClient - Closing the client now...\n\033[0m");
        return 32;
    } else {
        formatUserId(argv[1], userId);
        // strcpy(userId, argv[1]);
        printf("\033[33m\tCLIENT %s - Assigned User ID: %s\n\033[0m", userId, userId);
    }
    // Create a socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    } else {
        printf("\033[33m\tCLIENT %s - Connected to the server.\n\033[0m", userId);
    }

    // Set up the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_aton(SERVER_IP, &serverAddress.sin_addr);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) == -1) {
        perror("Error connecting to the server");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Create a thread for receiving messages from the server
//  if (pthread_create(&recv_thread, NULL, receive_messages, &clientSocket) != 0)
//  {
//    perror("Thread creation failed");
//    exit(EXIT_FAILURE);
//  }

    do {
        choice = showMenu();
        userChoiceOperations(choice, clientSocket, userId);
    } while (choice != 6);
    // Close the client socket
    // close the thread for the 'always listener'
//  pthread_cancel(recv_thread);
    close(clientSocket);
    pthread_exit(NULL);

    return 0;
}

int showMenu() {
    int choice = 0;
    bool continueClient = true;

    do {
        printf("\n\n<--- MENU ---> \n");
        printf("1. List Contacts\n");
        printf("2. Add User\n");
        printf("3. Delete user\n");
        printf("4. Send Messages\n");
        printf("5. Check Messages\n");
        printf("6. Close the client\n");
        printf("\tChoice: ");

        if (scanf("%d", &choice) != 1) {
            // If scanf fails to read an integer
            printf("\033[33m\tPlease enter a valid number.\n\033[0m");

            // Clearing the input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}

            continueClient = true;
        } else if (choice > 6 || choice < 1) {
            // If choice is outside the valid range
            printf("\033[33m\tPlease choose a valid Menu Option!\n\033[0m");
            continueClient = true;
        } else {
            continueClient = false;
        }
    } while (continueClient);

    return choice;
}

void userChoiceOperations(int choice, int clientSocket, char userId[4]) {

    switch (choice) {
        case 1:
            printf("\tYour Contacts:\n");
            // pthread_create(&tid, NULL, getContacts, &client_info);
            getContacts(userId, clientSocket);
            break;
        case 2:
            printf("\tAdding User:\n");
            // pthread_create(&tid, NULL, addUser, &client_info);
            addUser(userId, clientSocket);
            break;
        case 3:
            printf("\tDeleting User:\n");
            // pthread_create(&tid, NULL, deleteUser, &client_info);
            deleteUser(userId, clientSocket);
            break;
        case 4:
            printf("\tSending Messages:\n");
            // pthread_create(&tid, NULL, sendMessages, &client_info);
            sendMessages(userId, clientSocket);
            break;
        case 5:
            // pthread_create(&tid, NULL, checkMessages, &client_info);
            printf("\tChecking Messages:\n");
            checkMessages(userId, clientSocket);
            break;
        case 6:
            printf("\033[31m\tClosing the client now...\n\033[0m");
            break;
        default:
            break;
    }
}

//// MENU
// 1. get contacts
void getContacts(char userId[4], int clientSocket) {
    char command[MAX_BUFFER_SIZE];
    strcpy(command, userId);
    strcat(command, ":1");
//    puts("\nCLIENT - get contacts: ");
//    puts(command);
    send(clientSocket, command, strlen(command), 0);

    // Server response
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
//        printf("Server response: %s\n", buffer);

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
void addUser(char userId[4], int clientSocket) {
    UserInfo userInfo;
    bool isYourself = promptForUserInfo(userId, &userInfo);
    if (isYourself) {
        printf("\033[31m\tError: You cannot add yourself to your Contact List!\n\033[0m");
        return;
    }

    char message[MAX_BUFFER_SIZE];
    snprintf(message, MAX_BUFFER_SIZE, "%s:2:%s_%s_%s_%s", userId, userInfo.id, userInfo.phone, userInfo.name,
             userInfo.surname);
//    printf("Client - ADD User: %s\n", message);
    if (send(clientSocket, message, strlen(message), 0) < 0) {
        perror("Error sending data to server");
        return;
    }
//    printf("Client - Now waiting for the server response...\n");
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
//        printf("Server response: %s\n", buffer);
        if (strstr(buffer, "Contact already exists")) {
            printf("\033[31m\tError: %s! You cannot add an existing contact.\n\033[0m", buffer);
        } else {
            printf("\033[32mUser added successfully!\n\033[0m");
        }
    } else {
        perror("Error receiving data from server");
    }
}

// 3. delete user
void deleteUser(char userId[4], int clientSocket) {
    char contactId[4];

    printf("\tEnter the ID of the contact to be deleted: ");
    scanf("%3s", contactId);
    strcpy(contactId, formatUserIdOneInput(contactId));
    // Check if the entered ID is not the same as the client's own ID
    if (strcmp(contactId, userId) == 0) {
        printf("\033[31m\tError: Cannot delete own user ID.\n\033[0m");
        return;
    }

    // Format the command to send to the server
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s:3:%s", userId, contactId);
    send(clientSocket, command, strlen(command), 0);

    // Await and process server response
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        if (strcmp(buffer, "Contact not found") == 0) {
            printf("\033[31m\tError: %s\n\033[0m", buffer);
        } else {
            printf("\033[32m\tUser deleted successfully!\n\033[0m");
        }
    } else {
        perror("Error receiving data from server");
    }
}

// 4. send messages
void sendMessages(char userId[4], int clientSocket) {
    char input[255 + 4]; // Maximum message length + space for recipientId
    char recipientId[4];
    char message[255];

    // Clear the input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
    printf("\tEnter recipient's User ID followed by the message (e.g., '12 Hello there'): ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%3s %[^\n]", recipientId, message); // Read first 3 characters as ID, rest as message
    recipientId[3] = '\0'; // Ensure null-termination

    strcpy(recipientId, formatUserIdOneInput(recipientId));
    printf("Here is the recipient ID: %s\n", recipientId);
    if (strcmp(userId, recipientId) == 0) {
        printf("\033[31m\tError: You cannot send a message to yourself!\n\033[0m");
        return;
    }

    char initialCommand[MAX_BUFFER_SIZE];
    snprintf(initialCommand, sizeof(initialCommand), "%s:4:%s", userId, recipientId);
    send(clientSocket, initialCommand, strlen(initialCommand), 0);

    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        if (strcmp(buffer, "Same") == 0) {
            printf("\033[31m\tError: You cannot send a message to yourself!\n\033[0m");
            return;
        } else if (strstr(buffer, "Recipient not found")) {
            printf("\033[31m\tError: %s\n\033[0m", buffer);
            return;
        } else {
            printf("\033[32m\tFound: %s\n\033[0m", buffer);
        }
    } else {
        perror("Client - Error receiving data from server");
        return;
    }

    char fullMessage[MAX_BUFFER_SIZE];
    snprintf(fullMessage, sizeof(fullMessage), "mes:%s:", message);
    send(clientSocket, fullMessage, strlen(fullMessage), 0);

    char bufferFinal[MAX_BUFFER_SIZE];
    ssize_t received_bytes_final = recv(clientSocket, bufferFinal, sizeof(bufferFinal), 0);
    if (received_bytes_final > 0) {
        bufferFinal[received_bytes_final] = '\0'; // Null-terminate the received data
        if (strcmp(bufferFinal, "Terminate") == 0) {
            printf("\033[31m\tError: %s\n\033[0m", bufferFinal);
            return;
        }
        printf("\033[32m\tMessage sent successfully!\n\033[0m");
    } else {
        perror("Client - Error receiving data from server");
        return;
    }
}


// 5. check messages
void checkMessages(char userId[4], int clientSocket) {
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s:5", userId); // 5 is the code for checkMessages
    send(clientSocket, command, strlen(command), 0);

    // Receive the list of senders from the server
    char buffer[MAX_BUFFER_SIZE];
    ssize_t received_bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[received_bytes] = '\0';
    if (received_bytes > 0) {
//        printf("Server response: %s\n", buffer);
        if (strstr(buffer, "No message")) {
            printf("\033[33m\tThere are no messages yet!\n\033[0m");
            return;
        } else {
            printf("\tUser has messages, continuing...\n");
            printf("You have messages from the following users:\n");
            printf("%s", buffer);
            printf("\nWhose messages do you want to read? \nEnter User ID: ");
            char senderId[4];
            scanf("%3s", senderId);
            senderId[3] = '\0';  // Ensure string is null-terminated
            // Now, request the specific messages from the server
            // The server would need to handle this additional request
            snprintf(command, sizeof(command), "%s", senderId); // Additional info for specific user's messages
            send(clientSocket, command, strlen(command), 0);

            // Receive the messages one by one
            strcpy(senderId, formatUserIdOneInput(senderId));
            printf("\nMessages from User %s:\n\n", senderId);
            bool flagContinue = true;

            int counter = 0;

            while (flagContinue) {
                counter++;
                char takenMessage[MAX_BUFFER_SIZE] = {0};
                ssize_t receivedBytesMessage = recv(clientSocket, takenMessage, sizeof(takenMessage), 0);
                takenMessage[receivedBytesMessage] = '\0';
//                printf("Response: %s\n", takenMessage);
                if (strcmp(takenMessage, "Terminate") == 0) {
                    printf("\033[31m\tThere are no messages from this sender!\n\033[0m");
                    flagContinue = false;
                } else {
                    if (strcmp(takenMessage, "Finish") == 0) {
                        printf("\033[33m\tEnd of messages...\n\033[0m");
                        flagContinue = false;
                    } else {
                        printf("%s\n", takenMessage);
                        // confirm that the message is taken
                        char confirmMessage[MAX_BUFFER_SIZE];
                        snprintf(confirmMessage, sizeof(confirmMessage), "%s", "Message received");
                        send(clientSocket, confirmMessage, strlen(confirmMessage), 0);

                    }
                }
            }
//            printf("\tEnd of messages...\n");
            // Display the messages
        }
    } else {
        perror("Error receiving data from server");
        return;
    }

}


//// Utils
bool promptForUserInfo(char userId[4], UserInfo *userInfo) {
    printf("\nEnter ID, Mobile Phone, Name, Surname, in order:\n");

    printf("ID: ");
    scanf("%3s", userInfo->id);
    strcpy(userInfo->id, formatUserIdOneInput(userInfo->id));
    // Clear the input buffer
    while (getchar() != '\n');

    if (strcmp(userId, userInfo->id) == 0) {
        return true;
    }

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
    return false;
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

char *formatUserIdOneInput(const char *input) {
    char *userId = malloc(4 * sizeof(char));  // Allocate memory for userId
    if (!userId) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    if (strlen(input) == 2) {
        userId[0] = '0';
        userId[1] = input[0];
        userId[2] = input[1];
    } else if (strlen(input) == 1) {
        userId[0] = '0';
        userId[1] = '0';
        userId[2] = input[0];
    } else {
        strncpy(userId, input, 3);  // Copy first 3 characters
    }
    userId[3] = '\0';  // Ensure string is null-terminated

    return userId;
}
