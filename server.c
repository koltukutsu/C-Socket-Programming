// Network
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

// Threading
#include <pthread.h>

// Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8005
#define MAX_BUFFER_SIZE 1024

pthread_mutex_t userList_mutex = PTHREAD_MUTEX_INITIALIZER;
// messages
typedef struct message {
    char correspondentId[4];
    char message[255];

    struct message *nextMessage;
} Message;
// correspondent contact
typedef struct contact {
    char contactUserId[4];
    char phoneNumber[14];
    char name[30];
    char surname[15];
//    Message *messagesList;
    struct contact *nextContact;
} Contact;

// user
typedef struct user {
    char userId[4];
    Contact *contactsList;
    Message *messagesList;
    struct user *nextUser;
} User;

typedef struct server_t {
    int client_socket;
    User **userList;
} server_t;

enum Mode {
    SEND_CONTACTS = 1,
    ADD_USER = 2,
    DELETE_USER = 3,
    TAKE_MESSAGES = 4,
    CHECK_MESSAGE = 5,
    CLOSE_CLIENT = 6
};

//// Handle Operations
void *handleClient(void *args);

//// Menu Operations
void sendContacts(char *userId, User **userList, int client_socket);

void
addUserToContact(char userId[4], User **userList, int client_socket, ssize_t received_bytes,
                 char buffer[MAX_BUFFER_SIZE]);

void
deleteUser(char userId[4], User **userList, int client_socket, ssize_t received_bytes, char buffer[MAX_BUFFER_SIZE]);

void
takeMessages(char userId[4], User **userList, int client_socket, ssize_t received_bytes, char buffer[MAX_BUFFER_SIZE]);

void checkMessages(char userId[4], User **userList, int client_socket);

//// User Operations
void printContactInfo(const char *contactId, const char *phoneNumber, const char *name, const char *surname);

bool checkUserExists(char userId[4], User **userList);

void printAllUsers(User *userList);

void createNewUser(char userId[4], User **userList);

char *formatUserIdOneInput(const char *input);

void formatUserId(char *input, char *userId);

//// FILE & FOLDER Operations
void addMessageToFile(char recipientId[4], char senderId[4], char *message);

void addContactToFile(char userId[4], char *contactInfo);

void deleteContactFromFile(char userId[4], char *contactId);

void loadContactsFromFile(const char *filePath, Contact **contactsList);

void loadDatabase(User **userList);

void printFileContents(const char *filePath);

int main() {
    if (true) {
        char pwd[100];
        getcwd(pwd, sizeof(pwd));
        printf("Server - Current working directory: %s\n", pwd);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    pthread_t tid;
    User *userList = NULL;

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
    if (bind(server_socket, (struct sockaddr *) &server_address,
             sizeof(server_address)) == -1) {
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
    // loads the database from the local files
    loadDatabase(&userList);

    printf("\n SERVER - Server listening on port: %d\n", PORT);

    while (1) {
        // Accept a connection
        if ((client_socket =
                     accept(server_socket, (struct sockaddr *) &client_address,
                            &client_address_len)) == -1) {
            perror("Error accepting connection");
            continue;
        }
        server_t *serverInfo = (server_t *) malloc(sizeof(server_t));
        serverInfo->client_socket = client_socket;
        serverInfo->userList = &userList;
        printf("Connection accepted from %s\n", inet_ntoa(client_address.sin_addr));

        // Handle the client's request
        if (pthread_create(&tid, NULL, &handleClient, (void *) serverInfo) != 0) {
            perror("Error in creating thread");
            exit(1);
        }
        pthread_detach(tid);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}

//// Handle Operations
void *handleClient(void *args) {
    char userId[4];
    char buffer[MAX_BUFFER_SIZE];
    server_t *serverInfo = (server_t *) args;
    // Receive command from the client

    int flag = 1;
    while (flag) {
        ssize_t received_bytes =
                recv(serverInfo->client_socket, buffer, sizeof(buffer), 0);
        if (received_bytes < 0) {
            perror("Error receiving data from client");
            close(serverInfo->client_socket);
            pthread_detach(pthread_self());
        }
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        // if (strlen(buffer) == 3) {

        char mode;
        puts("\nSERVER - START CONNECTION");
        strncpy(userId, buffer, 3);
        userId[3] = '\0';

        char *endPtr;
        long userIdInt = strtol(userId, &endPtr, 10); // Base 10 for decimal numbers

        // Check if userId is not a number or contains additional characters after a number
        if (endPtr == userId || *endPtr != '\0') {
            puts("\nSERVER - User Id is not a valid integer...");
            puts("\nSERVER - The connection of this thread is closed");
            flag = 0;
//            close(serverInfo->client_socket);
//            free(serverInfo);
//            exit(EXIT_FAILURE);
            continue;
        } else {
            mode = buffer[4];
            printf("SERVER - Connected client Id: %s and mode: %c\n", userId, mode);
            char new[2];
            new[0] = mode;
            new[1] = '\0';
            // puts(new);

            // Check if the user exists in the user list
            bool userExists = checkUserExists(userId, serverInfo->userList);

            if (!userExists) {
                // If the user does not exist, create a new user
                puts("\n SERVER - User does not exist!");
                puts("\n SERVER - Creating the user...");
                createNewUser(userId, serverInfo->userList);

            } else {
                puts("\n SERVER - User exists, continuing...");
                pthread_mutex_lock(&userList_mutex);
                printAllUsers(*(serverInfo->userList));
                pthread_mutex_unlock(&userList_mutex);

            }
            // If it enters here, the mod and the userID is already given
            // so there is already a buffer present here
            // and client wants a response from the server...
            printf("\n\tClient/User Id: %s\n", userId);
            switch (mode) {
                case '1':
                    puts("\nSERVER - 1 - request to send Contacts");
                    sendContacts(userId, serverInfo->userList, serverInfo->client_socket);
                    break;
                case '2':
                    puts("\nSERVER - 2 - request to add User");
                    addUserToContact(userId, serverInfo->userList, serverInfo->client_socket, received_bytes, buffer);
                    break;
                case '3':
                    puts("\nSERVER - 3 - request to delete User");
                    deleteUser(userId, serverInfo->userList, serverInfo->client_socket, received_bytes, buffer);
                    break;
                case '4':
                    puts("\nSERVER - 4 - request to take/send Messages");
                    takeMessages(userId, serverInfo->userList, serverInfo->client_socket, received_bytes, buffer);
                    break;
                case '5':
                    puts("\nSERVER - 5 - request to check Messages");
                    checkMessages(userId, serverInfo->userList, serverInfo->client_socket);
                    break;
                case '6':
                    puts("\nSERVER - 6 - request to close client");
                    flag = 0;
                    break;
                default:
                    break;
            }
        }
    }
//        if (userIdInt < 0 || userIdInt > 999) {
//            puts("\nSERVER - User Id is out of range");
//            close(serverInfo->client_socket);
//            free(serverInfo);
//            exit(EXIT_FAILURE);
//        }


    // Close the connection
    close(serverInfo->client_socket);
    free(serverInfo);
}

//// Menu Operations
// Client -> 1. get contacts
void sendContacts(char userId[4], User **userList, int client_socket) {
    // Search for the user with the specified userId
    pthread_mutex_lock(&userList_mutex);
    User *currentUser = *userList;
    while (currentUser != NULL) {
        if (strcmp(currentUser->userId, userId) == 0) {
            // Found the user, now send their contacts
            Contact *currentContact = currentUser->contactsList;
            char contactsMessage[MAX_BUFFER_SIZE];
            memset(contactsMessage, 0, sizeof(contactsMessage));

            // Iterate through the contacts and append them to the message
            if (currentContact == NULL) {
                char *noContactMessage = "There is no contact";
                send(client_socket, noContactMessage, strlen(noContactMessage), 0);
                pthread_mutex_unlock(&userList_mutex);
                return;
            }
            while (currentContact != NULL) {
                // Format: ContactUserId:PhoneNumber:Name:Surname
                strcat(contactsMessage, currentContact->contactUserId);
                strcat(contactsMessage, ":");
                strcat(contactsMessage, currentContact->phoneNumber);
                strcat(contactsMessage, ":");
                strcat(contactsMessage, currentContact->name);
                strcat(contactsMessage, ":");
                strcat(contactsMessage, currentContact->surname);
                strcat(contactsMessage, "_");
                currentContact = currentContact->nextContact;
            }

            // Send the contacts message to the client
            puts("\nSERVER - 1 - send message: ");
            puts(contactsMessage);
            send(client_socket, contactsMessage, strlen(contactsMessage), 0);
            pthread_mutex_unlock(&userList_mutex);
            return;
        }

        currentUser = currentUser->nextUser;
    }

    // If the user was not found, send an appropriate message
    char notFoundMessage[MAX_BUFFER_SIZE];
    snprintf(notFoundMessage, sizeof(notFoundMessage), "User %s not found",
             userId);
    send(client_socket, notFoundMessage, strlen(notFoundMessage), 0);
    pthread_mutex_unlock(&userList_mutex);
}

// Client -> 2. add contact

void
addUserToContact(char userId[4], User **userList, int client_socket, ssize_t received_bytes,
                 char buffer[MAX_BUFFER_SIZE]) {
    pthread_mutex_lock(&userList_mutex);
//    char buffer[MAX_BUFFER_SIZE];
//    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (received_bytes < 0) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error receiving data from client");
        return;
    }

    buffer[received_bytes] = '\0'; // Null-terminate the received data

    // Parse the received data
    char contactId[4], phoneNumber[14], name[30], surname[15];
    char *token;
    const char delim[] = "_";
    puts("Server - Received data:");
    puts(buffer);
    puts("\n");
    // before coming to the extraction operations, the first 6 characters are deleted.
    memmove(buffer, buffer + 6, strlen(buffer));
    // Extract contactId
    token = strtok(buffer, delim);
    if (token != NULL) strncpy(contactId, token, sizeof(contactId) - 1);
    // add terminating character
    contactId[sizeof(contactId) - 1] = '\0';

    // Extract phoneNumber
    token = strtok(NULL, delim);
    if (token != NULL) strncpy(phoneNumber, token, sizeof(phoneNumber) - 1);
    phoneNumber[sizeof(phoneNumber) - 1] = '\0';

    // Extract name
    token = strtok(NULL, delim);
    if (token != NULL) strncpy(name, token, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    // Extract surname
    token = strtok(NULL, delim);
    if (token != NULL) strncpy(surname, token, sizeof(surname) - 1);
    surname[sizeof(surname) - 1] = '\0';
    // Check if the user exists, if not, create a new user
    User *currentUser = *userList;
    while (currentUser != NULL && strcmp(currentUser->userId, userId) != 0) {
        currentUser = currentUser->nextUser;
    }

    if (currentUser == NULL) { // User not found, create new
        createNewUser(userId, userList);
        currentUser = *userList;
    }
    // printf the taken information here in a function called printContactInfo
    printContactInfo(contactId, phoneNumber, name, surname);
    // control whether the contact is already in the contact list of the current user
    Contact *currentContact = currentUser->contactsList;
    while (currentContact != NULL) {
        if (strcmp(currentContact->contactUserId, contactId) == 0) {
            // Contact already exists
            char *errorMsg = "Contact already exists";
            send(client_socket, errorMsg, strlen(errorMsg), 0);
            pthread_mutex_unlock(&userList_mutex);
            return;
        }
        currentContact = currentContact->nextContact;
    }

    // Create a new contact and add it to the user's contact list
    Contact *newContact = (Contact *) malloc(sizeof(Contact));
    if (newContact == NULL) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error allocating memory for new contact");
        exit(EXIT_FAILURE);
    }

    strncpy(newContact->contactUserId, contactId, sizeof(newContact->contactUserId));
    strncpy(newContact->phoneNumber, phoneNumber, sizeof(newContact->phoneNumber));
    strncpy(newContact->name, name, sizeof(newContact->name));
    strncpy(newContact->surname, surname, sizeof(newContact->surname));
    // print the newContact
    printf("Server - New Contact: %s_%s_%s_%s\n", newContact->contactUserId, newContact->phoneNumber, newContact->name,
           newContact->surname);
    newContact->nextContact = currentUser->contactsList;
    currentUser->contactsList = newContact;

    // Write the contact to the user's contacts.txt file
    char contactInfo[75]; // Adjust the size as needed
    snprintf(contactInfo, sizeof(contactInfo), "%s_%s_%s_%s", contactId, phoneNumber, name, surname);
    addContactToFile(userId, contactInfo);

    // print me the pwd

    // Acknowledge the client
    char ackMessage[] = "Contact added successfully";
    send(client_socket, ackMessage, strlen(ackMessage), 0);
    pthread_mutex_unlock(&userList_mutex);
}

// Client -> 3. delete contact
void
deleteUser(char userId[4], User **userList, int client_socket, ssize_t received_bytes, char buffer[MAX_BUFFER_SIZE]) {
    pthread_mutex_lock(&userList_mutex);

    if (received_bytes < 0) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error receiving data from client");
        return;
    }

    char recipientId[4];
    char *token = strtok(buffer, ":");
    if (token != NULL) { token = strtok(NULL, ":"); } // Skip mode
    if (token != NULL) {
        token = strtok(NULL, ":");
        if (token != NULL) {
            strncpy(recipientId, token, sizeof(recipientId) - 1);
            recipientId[sizeof(recipientId) - 1] = '\0'; // Ensure null-termination
        }
    }

    // Find the current user and delete the contact
    User *currentUser = *userList;
    while (currentUser != NULL && strcmp(currentUser->userId, userId) != 0) {
        currentUser = currentUser->nextUser;
    }

    if (currentUser == NULL) {
        char *errorMsg = "User not found";
        send(client_socket, errorMsg, strlen(errorMsg), 0);
        pthread_mutex_unlock(&userList_mutex);
        return;
    }

    Contact **contactPtr = &(currentUser->contactsList);
    while (*contactPtr != NULL) {
        if (strcmp((*contactPtr)->contactUserId, recipientId) == 0) {
            Contact *temp = *contactPtr;
            *contactPtr = (*contactPtr)->nextContact;
            free(temp);
            deleteContactFromFile(userId, recipientId);
            char *successMsg = "Contact deleted successfully";
            send(client_socket, successMsg, strlen(successMsg), 0);
            pthread_mutex_unlock(&userList_mutex);
            return;
        }
        contactPtr = &((*contactPtr)->nextContact);
    }

    char *notFoundMsg = "Contact not found";
    send(client_socket, notFoundMsg, strlen(notFoundMsg), 0);
    pthread_mutex_unlock(&userList_mutex);
}

// Client -> 4. take messages
void
takeMessages(char userId[4], User **userList, int client_socket, ssize_t received_bytes, char buffer[MAX_BUFFER_SIZE]) {
    pthread_mutex_lock(&userList_mutex);
//    char buffer[MAX_BUFFER_SIZE];
//    recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes < 0) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error receiving data from client");
        return;
    }

    char *token;
    char recipientId[4];

    // First call to strtok gets the first part (userID)
    token = strtok(buffer, ":");
    if (token != NULL) {
        // Second call to strtok gets the mode (which is '4' in this case)
        token = strtok(NULL, ":");
    }

    if (token != NULL) {
        // Third call to strtok gets the recipient's user ID
        token = strtok(NULL, ":");
        if (token != NULL) {
            strncpy(recipientId, token, sizeof(recipientId) - 1);
            recipientId[sizeof(recipientId) - 1] = '\0'; // Ensure null-termination
        }
    }
    // control whether the contact id is in the contact list of the current user
    bool isRecipientExists = false;
    User *currentUser = *userList;
    bool flag = true;
    while (currentUser != NULL && flag) {
        if (strcmp(currentUser->userId, userId) == 0) {
            Contact *currentContact = currentUser->contactsList;
            int flag2 = true;
            while (currentContact != NULL && flag2) {
                if (strcmp(currentContact->contactUserId, recipientId) == 0) {
                    isRecipientExists = true;
                    flag2 = false;
                }
                currentContact = currentContact->nextContact;
            }
            flag = false;
        }
        currentUser = currentUser->nextUser;
    }
    if (isRecipientExists) {
        // Recipient found. Send acknowledgment and receive message
        char ack[] = "Recipient found";
        puts("\t\tSent the message, waiting client response...");
        send(client_socket, ack, strlen(ack), 0);

        puts("\t\tGet the message, continuing...");
        // Receive the message from the client
        char messageBuffer[MAX_BUFFER_SIZE];
        ssize_t receivedBytesMessage = recv(client_socket, messageBuffer, sizeof(messageBuffer), 0);

        if (receivedBytesMessage < 0) {
            perror("Error receiving data from client");
            pthread_mutex_unlock(&userList_mutex);
            return;
        }

        messageBuffer[receivedBytesMessage] = '\0'; // Ensure null-termination
        strcpy(recipientId, formatUserIdOneInput(recipientId));
//        formatUserId(recipientId, recipientId);

        // TODO: Add the message to the recipient's messages list
        // A recipient is found with the recipientId, so the message is added to the recipient's message list.
        User *recipientUser = *userList;
        bool flagOut = false;
        while (recipientUser != NULL && !flagOut) {
            puts("\n comparison");
            puts(recipientUser->userId);
            puts(recipientId);
            if (strcmp(recipientUser->userId, recipientId) == 0) {
                // Found the recipient user
                Message *newMessage = (Message *) malloc(sizeof(Message));
                if (newMessage == NULL) {
                    // Handle memory allocation error
                    perror("Error allocating memory for new message");
                    flagOut = true;
                } else {
                    // Copy the message details to the new message node
                    strncpy(newMessage->correspondentId, userId, sizeof(newMessage->correspondentId) - 1);
                    newMessage->correspondentId[sizeof(newMessage->correspondentId) - 1] = '\0';
                    strncpy(newMessage->message, messageBuffer, sizeof(newMessage->message) - 1);
                    newMessage->message[sizeof(newMessage->message) - 1] = '\0';

                    // Insert the new message at the beginning of the recipient's messages list
                    newMessage->nextMessage = recipientUser->messagesList;
                    recipientUser->messagesList = newMessage;

                    // Acknowledge that the message has been added
                    char confirmMsg[] = "Message added successfully";
                    send(client_socket, confirmMsg, strlen(confirmMsg), 0);
                    // Write the message to the recipient's messages.txt file using the addMessageToFile function
                    char message[255]; // Adjust the size as needed
                    if (strlen(messageBuffer) > 255) {
                        // Message is too long, reducing the size of it
                        printf("\033[33m\t\tMessage is too long, truncating to 255 characters\n\033[0m");
                        messageBuffer[255] = '\0';
                    }
                    strncpy(message, messageBuffer, sizeof(newMessage->message) - 1);
                    addMessageToFile(recipientId, userId, message);
                    flagOut = true;
                }


            } else {
                puts("Recipient not found");

            }
            recipientUser = recipientUser->nextUser;
        }

    } else {
        // Recipient not found in contact list
        char errorMsg[] = "Recipient not found";
        send(client_socket, errorMsg, strlen(errorMsg), 0);
    }
    pthread_mutex_unlock(&userList_mutex);
}

//  Client -> 5. check messages
void checkMessages(char userId[4], User **userList, int client_socket) {
    pthread_mutex_lock(&userList_mutex);

    // Find the user in the userList
    User *currentUser = *userList;
    while (currentUser != NULL && strcmp(currentUser->userId, userId) != 0) {
        currentUser = currentUser->nextUser;
    }

    if (currentUser == NULL) {
        char *msg = "User not found\n";
        send(client_socket, msg, strlen(msg), 0);
    } else {
        // Iterate through the user's messages and collect unique sender IDs
        char senders[MAX_BUFFER_SIZE] = {0};
        Message *message = currentUser->messagesList;
        if (message != NULL) {
            // there are messages
            while (message != NULL) {
                if (strstr(senders, message->correspondentId) == NULL) {
                    strcat(senders, "User ");
                    strcat(senders, message->correspondentId);
                    strcat(senders, "\n");
                }
                message = message->nextMessage;
            }

            // Send the list of senders back to the client
            puts("\t\tSending the message senders list...");
            send(client_socket, senders, strlen(senders), 0);

            // Receive the sender ID from the client
            char senderId[4];
            ssize_t receivedBytes = recv(client_socket, senderId, sizeof(senderId), 0);
            puts("\t\tReceived the sender ID from the client...");
            if (receivedBytes < 0) {
                perror("Error receiving data from client");
                pthread_mutex_unlock(&userList_mutex);
                return;
            }
            senderId[receivedBytes] = '\0'; // Ensure null-termination
            strcpy(senderId, formatUserIdOneInput(senderId));

            message = currentUser->messagesList;
            puts("\t\tSending the messages...");
            puts("\t\tClient ID:");
            puts(currentUser->userId);
            puts("\t\tWanted Sender ID:");
            puts(senderId);
            char messageToSend[MAX_BUFFER_SIZE];
            bool flagSentMessage = false;
            while (message != NULL) {
                if (strcmp(senderId, message->correspondentId) == 0) {
//                    strcpy(messageToSend, "Message: ");
//                    strcpy(messageToSend, "\t - ");
                    strcat(messageToSend, message->message);
                    strcat(messageToSend, "\n");
                    // terminate the message string
                    messageToSend[strlen(messageToSend)] = '\0';
                    // send the message to the client
                    send(client_socket, messageToSend, strlen(messageToSend), 0);
                    flagSentMessage = true;
                    // control whether the message is sent or not
                    char confirmMsg[MAX_BUFFER_SIZE];
                    ssize_t receivedBytesConfirm = recv(client_socket, confirmMsg, sizeof(confirmMsg), 0);
                    if (receivedBytesConfirm < 0) {
                        perror("Error receiving data from client");
                        pthread_mutex_unlock(&userList_mutex);
                        return;
                    }
                    confirmMsg[receivedBytesConfirm] = '\0'; // Ensure null-termination
                    printf("\t\tReceived the confirmation message from the client: %s\n", confirmMsg);
                    if (strcmp(confirmMsg, "Message received") == 0) {
                        // TODO: Message received, delete it from the user's messages list
                        // no deletion!!!
                        // Message *temp = message;
                        // message = message->nextMessage;
                        // free(temp);
                        printf("\033[32m\t\tMessage is sent successfully.\n\033[0m");
                    } else {
                        // Message not received, continue to the next message
                        // message = message->nextMessage;
                        printf("\033[31m\t\tUnable to send the message!\n\033[0m");
                    }
                }
                message = message->nextMessage;

            }
            if (flagSentMessage) {
                char *msgTerminate = "Finish";
                send(client_socket, msgTerminate, strlen(msgTerminate), 0);
                puts("\t\tFinished the messages...");
            } else {
                char *msgTerminate = "Terminate";
                send(client_socket, msgTerminate, strlen(msgTerminate), 0);
                puts("\t\tThere is no Message Sender as such...");
            }

        } else {
            // there is no message
            char *msg = "No message";
            send(client_socket, msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&userList_mutex);
}

//// User Operations
void printContactInfo(const char *contactId, const char *phoneNumber, const char *name, const char *surname) {
    printf("Print Contact Info: Contact ID: %s, Phone Number: %s, Name: %s, Surname: %s\n", contactId, phoneNumber,
           name, surname);
}

void printAllUsers(User *userList) {
    User *currentUser = userList;

    printf("\033[32m\nList of all users:\n\033[0m");
    while (currentUser != NULL) {
        printf("\tUser ID: %s\n", currentUser->userId);
        currentUser = currentUser->nextUser;
    }
}


bool checkUserExists(char userId[4], User **userList) {
    pthread_mutex_lock(&userList_mutex);
    User *currentUser = *userList;
    while (currentUser != NULL) {
        if (strcmp(currentUser->userId, userId) == 0) {
            // User exists
            pthread_mutex_unlock(&userList_mutex);
            return true;
        }
        currentUser = currentUser->nextUser;
    }
    // User not found
    pthread_mutex_unlock(&userList_mutex);

    return false;
}

void createNewUser(char userId[4], User **userList) {
    pthread_mutex_lock(&userList_mutex);
    User *newUser = (User *) malloc(sizeof(User));
    if (newUser == NULL) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error allocating memory for new user");
        exit(EXIT_FAILURE);
    }

    // Initialize the new user's fields
    strncpy(newUser->userId, userId, sizeof(newUser->userId));
    newUser->userId[sizeof(newUser->userId) - 1] = '\0';
    newUser->contactsList = NULL; // Initialize the contacts list to be empty
    newUser->messagesList = NULL; // Initialize the messages list to be empty
    newUser->nextUser = NULL;

    if (*userList == NULL) {
        // If the user list is empty, set the new user as the head of the list
        *userList = newUser;
    } else {
        // Find the last user in the list and append the new user to it
        User *lastUser = *userList;
        while (lastUser->nextUser != NULL) {
            lastUser = lastUser->nextUser;
        }
        lastUser->nextUser = newUser;
    }

    // Create a 'database' directory if it doesn't exist
    char databaseFolder[] = "./database";
    if (mkdir(databaseFolder, 0755) != 0) {
        if (errno != EEXIST) {
            pthread_mutex_unlock(&userList_mutex);
            perror("Error creating database directory");
            exit(EXIT_FAILURE);
        }
    }

    // Create a directory for the new user inside the 'database' folder
    char userFolder[50]; // Adjust the buffer size as needed
    snprintf(userFolder, sizeof(userFolder), "%s/user_%s", databaseFolder, userId);
    if (mkdir(userFolder, 0755) != 0) {
        if (errno != EEXIST) {
            pthread_mutex_unlock(&userList_mutex);
            perror("Error creating user directory");
            exit(EXIT_FAILURE);
        }
    }

    // Create files for messages and contacts in the user's directory
    char messagesFilePath[70], contactsFilePath[70];
    snprintf(messagesFilePath, sizeof(messagesFilePath), "%s/messages.txt", userFolder);
    snprintf(contactsFilePath, sizeof(contactsFilePath), "%s/contacts.txt", userFolder);

    FILE *messagesFile = fopen(messagesFilePath, "w");
    if (messagesFile == NULL) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error creating messages file");
        exit(EXIT_FAILURE);
    }
    fclose(messagesFile);

    FILE *contactsFile = fopen(contactsFilePath, "w");
    if (contactsFile == NULL) {
        pthread_mutex_unlock(&userList_mutex);
        perror("Error creating contacts file");
        exit(EXIT_FAILURE);
    }
    fclose(contactsFile);
    pthread_mutex_unlock(&userList_mutex);
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

//// FILE & FOLDER Operations
void addMessageToFile(char recipientId[4], char senderId[4], char *message) {
    char messagesFilePath[70];
    snprintf(messagesFilePath, sizeof(messagesFilePath), "./database/user_%s/messages.txt", recipientId);

    FILE *messagesFile = fopen(messagesFilePath, "a");
    if (messagesFile == NULL) {
        perror("Error opening messages file");
        exit(EXIT_FAILURE);
    }

    // Writing the message in the format "{senderId}:{message}"
    fprintf(messagesFile, "%s:%s\n", senderId, message);
    fclose(messagesFile);
}

void addContactToFile(char userId[4], char *contactInfo) {
    char contactsFilePath[70];
    snprintf(contactsFilePath, sizeof(contactsFilePath), "./database/user_%s/contacts.txt", userId);

    FILE *contactsFile = fopen(contactsFilePath, "a");
    if (contactsFile == NULL) {
        perror("Error opening contacts file");
        exit(EXIT_FAILURE);
    }

    fprintf(contactsFile, "%s\n", contactInfo);
    fclose(contactsFile);
}

void deleteContactFromFile(char userId[4], char *contactId) {
    char contactsFilePath[70];
    snprintf(contactsFilePath, sizeof(contactsFilePath), "./database/user_%s/contacts.txt", userId);

    // Temporary file to store updated contacts
    char tempFilePath[75]; // Increased size to accommodate ".temp"
    snprintf(tempFilePath, sizeof(tempFilePath), "%s.temp", contactsFilePath);

    FILE *contactsFile = fopen(contactsFilePath, "r");
    FILE *tempFile = fopen(tempFilePath, "w");

    if (contactsFile == NULL || tempFile == NULL) {
        perror("Error opening files");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), contactsFile) != NULL) {
        if (strstr(line, contactId) == NULL) {
            fputs(line, tempFile);
        }
    }

    fclose(contactsFile);
    fclose(tempFile);

    // Replace old contacts file with new temp file
    remove(contactsFilePath);
    rename(tempFilePath, contactsFilePath);
}

//// Starting Operations
void loadContactsFromFile(const char *filePath, Contact **contactsList) {
    printf("\n\t\tCONTACTS:");
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening contacts file");
        return;
    }

    // Check if the file is empty
//    fseek(file, 0, SEEK_END); // Go to end of file
//    if (ftell(file) == 0) {
//        printf("\nFile is empty: %s", filePath);
//        fclose(file);
//        return;
//    }
//    rewind(file);

    char line[256];
    bool isEmpty = true; // Flag to check if the file is empty

    while (fgets(line, sizeof(line), file) != NULL) {
        isEmpty = false; // File has content

        Contact *newContact = (Contact *) malloc(sizeof(Contact));
        if (newContact == NULL) {
            perror("Error allocating memory for new contact");
            continue;
        }

        // Parse the line based on the expected format
        sscanf(line, "%3[^_]_%13[^_]_%29[^_]_%14[^\n]",
               newContact->contactUserId, newContact->phoneNumber,
               newContact->name, newContact->surname);

        newContact->nextContact = *contactsList; // Add to the front of the contacts list
        *contactsList = newContact;
        printf("\n\t\t\tContact ID: %s, Phone Number: %s, Name: %s, Surname: %s",
               newContact->contactUserId, newContact->phoneNumber,
               newContact->name, newContact->surname);
    }

    if (isEmpty) {
        printf("\n\t\t\tNo contacts found in file: %s", filePath);
    }

    fclose(file);
}

void loadMessagesFromFile(const char *filePath, Message **messagesList) {
    printf("\n\t\tMESSAGES:");
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening messages file");
        return;
    }

    char line[256];
    bool isEmpty = true;

    while (fgets(line, sizeof(line), file)) {
        isEmpty = false;
        Message *newMessage = (Message *) malloc(sizeof(Message));
        if (newMessage == NULL) {
            perror("Error allocating memory for new message");
            continue;
        }

        // Assuming each line in messages.txt is formatted as "correspondentId:message"
        sscanf(line, "%3[^:]:%254[^\n]", newMessage->correspondentId, newMessage->message);
        newMessage->nextMessage = *messagesList;
        *messagesList = newMessage;
        printf("\n\t\t\tCorrespondent ID: %s, Message: %s", newMessage->correspondentId, newMessage->message);
    }

    if (isEmpty) {
        printf("\n\t\t\tNo messages found in file: %s", filePath);
    }

    fclose(file);
}

void loadDatabase(User **userList) {
    DIR *dir;
    struct dirent *entry;

    // Try to open the directory
    dir = opendir("./database");
    if (!dir) {
        printf("\033[32mSERVER - Notice: This is a fresh start.\n\033[0m");
        return;
    } else {
        printf("\033[33mSERVER - Loading: Database is loading...\n\033[0m");
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strncmp(entry->d_name, "user_", 5) == 0) {
            char userId[4];
            strncpy(userId, entry->d_name + 5, 3);

            userId[3] = '\0';
//            puts("\t\t");
//            puts(userId);

            User *newUser = (User *) malloc(sizeof(User));
            if (newUser == NULL) {
                perror("Error allocating memory for new user");
                continue;
            }
//            strncpy(newUser->userId, userId, sizeof(newUser->userId) - 1);
            strcpy(newUser->userId, userId);
            newUser->contactsList = NULL;
            newUser->messagesList = NULL; // Initialize messages list
            newUser->nextUser = *userList;
            *userList = newUser;

            char contactsFilePath[512], messagesFilePath[512];
            snprintf(contactsFilePath, sizeof(contactsFilePath), "./database/%s/contacts.txt", entry->d_name);
            snprintf(messagesFilePath, sizeof(messagesFilePath), "./database/%s/messages.txt", entry->d_name);

//            printFileContents(contactsFilePath);
//            printFileContents(messagesFilePath);
            printf("\n\tSERVER - Data of %s", userId);
//            printf("\n \tContact file path: %s", contactsFilePath);
//            printf("\n \tMessage file path: %s", messagesFilePath);
            loadContactsFromFile(contactsFilePath, &(newUser->contactsList));
            loadMessagesFromFile(messagesFilePath, &(newUser->messagesList));
            printf("\n");
        }
    }
    printAllUsers(*userList);

    closedir(dir);
}


void printFileContents(const char *filePath) {
    printf("File contents of %s:\n", filePath);
    FILE *file = fopen(filePath, "r"); // Open the file for reading
    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[1024]; // Buffer for storing file content

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer); // Print each line of the file
    }

    fclose(file); // Close the file
}