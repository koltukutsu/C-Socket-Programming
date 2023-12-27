// Network
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

// Threading
#include <pthread.h>

// Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 9005
#define MAX_BUFFER_SIZE 1024

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
  char name[15];
  char surname[15];
  Message *messagesList;
  struct contact *nextContact;
} Contact;

// user
typedef struct user {
  char userId[4];
  Contact *contactsList;

  struct user *nextUser;
} User;

typedef struct server_t {
  int client_socket;
  User **userList;
} server_t;

//// Handle Operations
void *handleClient(void *args);

//// Menu Operations
void sendContacts(char *userId, User **userList, int client_socket);
void addUser(char userId[4], User **userList);
void deleteUser(char userId[4], User **userList);
void takeMessages(char userId[4], User **userList);
void checkMessage(char userId[4], User **userList);

//// User Operations
bool checkUserExists(char userId[4], User **userList);
void printAllUsers(User *userList);

//// FILE & FOLDER Operations
void addMessageToFile(char userId[4], char *message);
void addContactToFile(char userId[4], char *contactInfo);
void deleteContactFromFile(char userId[4], char *contactId);


int main() {
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
  if (bind(server_socket, (struct sockaddr *)&server_address,
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

  printf("Server listening on port: %d\n", PORT);

  while (1) {
    // Accept a connection
    if ((client_socket =
             accept(server_socket, (struct sockaddr *)&client_address,
                    &client_address_len)) == -1) {
      perror("Error accepting connection");
      continue;
    }
    server_t *serverInfo = (server_t *)malloc(sizeof(server_t));
    serverInfo->client_socket = client_socket;
    serverInfo->userList = &userList;
    printf("Connection accepted from %s\n", inet_ntoa(client_address.sin_addr));

    // Handle the client's request
    if (pthread_create(&tid, NULL, &handleClient, (void *)serverInfo) != 0) {
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
  server_t *serverInfo = (server_t *)args;
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
      addUser(userId, serverInfo->userList);
    } else {
      puts("\n SERVER - User exists, continuing...");
    }
    printAllUsers(*(serverInfo->userList));
    switch (mode) {
    case '1':
      puts("\nSERVER - 1 - request to send Contacts");
      sendContacts(userId, serverInfo->userList, serverInfo->client_socket);
      break;
    case '2':
      puts("\nSERVER - 2 - request to add User");
      addUser(userId, serverInfo->userList);
      break;
    case '3':
      puts("\nSERVER - 3 - request to delete User");
      deleteUser(userId, serverInfo->userList);
      break;
    case '4':
      puts("\nSERVER - 4 - request to take/send Messages");
      takeMessages(userId, serverInfo->userList);
      break;
    case '5':
      puts("\nSERVER - 5 - request to check Messages");
      checkMessage(userId, serverInfo->userList);
      break;
    case '6':
      puts("\nSERVER - 6 - request to close client");
      flag = 0;
      break;
    default:
      break;
    }
  }
  // Close the connection
  close(serverInfo->client_socket);
  free(serverInfo);
}

//// Menu Operations
void sendContacts(char userId[4], User **userList, int client_socket) {
  // Search for the user with the specified userId
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
      return;
    }

    currentUser = currentUser->nextUser;
  }

  // If the user was not found, send an appropriate message
  char notFoundMessage[MAX_BUFFER_SIZE];
  snprintf(notFoundMessage, sizeof(notFoundMessage), "User %s not found",
           userId);
  send(client_socket, notFoundMessage, strlen(notFoundMessage), 0);
}

void addUser(char userId[4], User **userList) {
    User *newUser = (User *)malloc(sizeof(User));
    if (newUser == NULL) {
        perror("Error allocating memory for new user");
        exit(EXIT_FAILURE);
    }

    // Initialize the new user's fields
    strncpy(newUser->userId, userId, sizeof(newUser->userId));
    newUser->userId[sizeof(newUser->userId) - 1] = '\0';
    newUser->contactsList = NULL; // Initialize the contacts list to be empty
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
            perror("Error creating database directory");
            exit(EXIT_FAILURE);
        }
    }

    // Create a directory for the new user inside the 'database' folder
    char userFolder[50]; // Adjust the buffer size as needed
    snprintf(userFolder, sizeof(userFolder), "%s/user_%s", databaseFolder, userId);
    if (mkdir(userFolder, 0755) != 0) {
        if (errno != EEXIST) {
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
        perror("Error creating messages file");
        exit(EXIT_FAILURE);
    }
    fclose(messagesFile);

    FILE *contactsFile = fopen(contactsFilePath, "w");
    if (contactsFile == NULL) {
        perror("Error creating contacts file");
        exit(EXIT_FAILURE);
    }
    fclose(contactsFile);
}


void deleteUser(char userId[4], User **userList) {

  User *currentUser = *userList;
  if (currentUser != NULL) {
    // there is already record
    bool foundClient = false;
    while (currentUser->nextUser != NULL) {
      if (strcmp(currentUser->userId, userId) == 0) {
        foundClient = true;
      } else {
        currentUser = currentUser->nextUser;
      }
    }
    // the client is saved
    if (foundClient) {
      // add the user
      Contact *contactsList = currentUser->contactsList;
    } else {
      User *user = (User *)malloc(sizeof(User));
      currentUser->nextUser = user;
    }
  } else {
    // there is no record
    printf("There is no user");
  }
}

void takeMessages(char userId[4], User **userList) {

  User *currentUser = *userList;
  if (currentUser != NULL) {
    // there is already record
    bool foundClient = false;
    while (currentUser->nextUser != NULL) {
      if (strcmp(currentUser->userId, userId) == 0) {
        foundClient = true;
      } else {
        currentUser = currentUser->nextUser;
      }
    }
    // the client is saved
    if (foundClient) {
      // add the user
      Contact *contactsList = currentUser->contactsList;
    } else {
      User *user = (User *)malloc(sizeof(User));
      currentUser->nextUser = user;
    }
  } else {
    // there is no record
    printf("There is no user");
  }
}

void checkMessage(char userId[4], User **userList) {

  User *currentUser = *userList;
  if (currentUser != NULL) {
    // there is already record
    bool foundClient = false;
    while (currentUser->nextUser != NULL) {
      if (strcmp(currentUser->userId, userId) == 0) {
        foundClient = true;
      } else {
        currentUser = currentUser->nextUser;
      }
    }
    // the client is saved
    if (foundClient) {
      // add the user
      Contact *contactsList = currentUser->contactsList;
    } else {
      User *user = (User *)malloc(sizeof(User));
      currentUser->nextUser = user;
    }
  } else {
    // there is no record
    printf("There is no user");
  }
}

//// User Operations
void printAllUsers(User *userList) {
  User *currentUser = userList;

  printf("List of all users:\n");

  while (currentUser != NULL) {
    printf("User ID: %s\n", currentUser->userId);
    currentUser = currentUser->nextUser;
  }
}


bool checkUserExists(char userId[4], User **userList) {
  User *currentUser = *userList;
  while (currentUser != NULL) {
    if (strcmp(currentUser->userId, userId) == 0) {
      // User exists
      return true;
    }
    currentUser = currentUser->nextUser;
  }
  // User not found
  return false;
}

//// FILE & FOLDER Operations
void addMessageToFile(char userId[4], char *message) {
    char messagesFilePath[70];
    snprintf(messagesFilePath, sizeof(messagesFilePath), "./database/user_%s/messages.txt", userId);

    FILE *messagesFile = fopen(messagesFilePath, "a");
    if (messagesFile == NULL) {
        perror("Error opening messages file");
        exit(EXIT_FAILURE);
    }

    fprintf(messagesFile, "%s\n", message);
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
    char tempFilePath[70];
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
