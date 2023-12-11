#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6012
#define MAX_BUFFER_SIZE 1024

// messages
typedef struct message {
  char correspondentId[4];
  char message[255];

  struct message *nextMessage;
} Message;
// correspondent contact
typedef struct contact {
  char contactUserId[10];
  char phoneNumber[14];
  char name[15];
  char surname[15];

  struct contact *nextContact;
} Contact;

// user
typedef struct user {
  char userId[4];
  Contact *contactsList;
  Message *messagesList;

  struct user *nextUser;
} User;

void handleClient(int client_socket, User **userList, char *userId);
void sendContacts(char *userId, User **userList, int client_socket);
void addUser(char userId[4], User **userList);
void deleteUser(char userId[4], User **userList);
void takeMessages(char userId[4], User **userList);
void checkMessage(char userId[4], User **userList);

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_len = sizeof(client_address);
  User *userList = NULL;
  char userId[4];
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

    printf("Connection accepted from %s\n", inet_ntoa(client_address.sin_addr));

    // Handle the client's request
    handleClient(client_socket, &userList, userId);
  }

  // Close the server socket
  close(server_socket);

  return 0;
}

void handleClient(int client_socket, User **userList, char userId[4]) {
  char buffer[MAX_BUFFER_SIZE];

  // Receive command from the client
  ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
  if (received_bytes < 0) {
    perror("Error receiving data from client");
    close(client_socket);
    return;
  }
  buffer[received_bytes] = '\0'; // Null-terminate the received data
  // if (strlen(buffer) == 3) {
  if (false) {
    strncpy(userId, buffer, 3);

    // TODO: using the user id, create a new User
    printf("SERVER - Used id is taken: %s\nServer - notifying the user...\n",
           userId);
    const char *message = "\tServer accepted the user id.";
    send(client_socket, message, strlen(message), 0);
  } else {
    char mode;
    puts("\nSecond way");
    strncpy(userId, buffer, 3);
    mode = buffer[4];
    printf("SERVER - Connected client Id: %s and mode: %c", userId, mode);
    char new[2];
    new[0] = mode;
    new[1] = '\0';
    puts(new);
    switch (mode) {
    case '1':
      puts("\nSERVER - 1 - request to send Contacts");
      sendContacts(userId, userList, client_socket);
      break;
    case '2':
      puts("\nSERVER - 2 - request to add User");
      addUser(userId, userList);
      break;
    case '3':
      puts("\nSERVER - 3 - request to delete User");
      deleteUser(userId, userList);
      break;
    case '4':
      puts("\nSERVER - 4 - request to take/send Messages");
      takeMessages(userId, userList);
      break;
    case '5':
      puts("\nSERVER - 5 - request to check Messages");
      checkMessage(userId, userList);
      break;
    default:
      break;
    }

    // if (strncmp(buffer, "takeContacts", 12) == 0) {
    //   char userId[4];
    //   int pos = 13;
    //   strncpy(userId, buffer + (pos - 1), 3);
    //   // take the user id from the buffer
    //   sendContacts(userId, userList);
    // } else if (strncmp(buffer, "addUser", 7) == 0) {
    //   addUser(userId, userList);
    // } else if (strncmp(buffer, "deleteUser", 10) == 0) {

    // } else if (strncmp(buffer, "sendMessages", 12) == 0) {

    // } else if (strncmp(buffer, "checkMessages", 13) == 0) {
    // }

    // if (strcmp(buffer, "send") == 0) {
    //   // If the command is "send," send a message to the client
    //   const char *message = "Hello from the server!";
    //   send(client_socket, message, strlen(message), 0);
    // } else if (strcmp(buffer, "take") == 0) {
    //   // If the command is "take," receive a message and save it to a file
    //   received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    //   if (received_bytes < 0) {
    //     perror("Error receiving data from client");
    //     close(client_socket);
    //     return;
    //   }
    //   buffer[received_bytes] = '\0'; // Null-terminate the received data

    //   // Save the message to a file named "output.txt"
    //   FILE *file = fopen("output.txt", "w");
    //   if (file == NULL) {
    //     perror("Error opening file");
    //     close(client_socket);
    //     return;
    //   }
    //   fprintf(file, "%s", buffer);
    //   fclose(file);
    // }

    // Close the connection
    close(client_socket);
  }
}
void sendContacts(char userId[4], User **userList, int client_socket) {
  char message[MAX_BUFFER_SIZE];
  char delimiterContact = ':';
  char delimeterFields = '_';
  

  User *currentUser = *userList;
  if (currentUser != NULL) {
    // there is already record
    strcpy(message, "The user is present!");
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
    strcpy(message, "There is no user as such!");
    puts("There is no user");
  
  }
  send(client_socket, message, strlen(message), 0);
  return;
}

void addUser(char userId[4], User **userList) {
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
    User *user = (User *)malloc(sizeof(User));
    currentUser = user;
    strcpy(userId, currentUser->userId);
  }
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