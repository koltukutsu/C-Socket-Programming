#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 3000
#define MAX_BUFFER_SIZE 1024

void send_command(int socket, const char *command, const char *message) {
  // Send the command to the server
  send(socket, command, strlen(command), 0);

  // If the command is "take," send the message as well
  if (strcmp(command, "take") == 0) {
    send(socket, message, strlen(message), 0);
  }
}

int showMenu();
void userChoiceOperations(int choice, int client_socket, char *userId);
void getContacts(char *userId, int client_socket);
void addUser(int client_socket);
void deleteUser(int client_socket);
void sendMessages(int client_socket);
void checkMessages(int client_socket);

int main(int argc, char *argv[]) {
  int client_socket;
  int choice;
  struct sockaddr_in server_address;
  int flag = 0;
  char userId[4];
  // server id control
  if (argc != 2) {
    printf("CLIENT - Give a user id only as an argument.\n");
    printf("CLIENT - Program is closing...");
    return 32;
  } else {
    printf("%s\n", argv[1]);
    strcpy(userId, argv[1]);
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
  if (connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address)) == -1) {
    perror("Error connecting to the server");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  do {
    // Get user input for the command
    choice = showMenu();
    userChoiceOperations(choice, client_socket, userId);
    // Get user input for the message (if the command is "take")
    char message[MAX_BUFFER_SIZE];
    if (strcmp(command, "take") == 0) {
      printf("Enter message to be saved: ");
      fgets(message, sizeof(message), stdin);
      message[strcspn(message, "\n")] = '\0'; // Remove newline character
    } else {
      // If the command is "send," set an empty message
      message[0] = '\0';
    }
  } while (choice != 6);
  // Close the client socket
  close(client_socket);

  return 0;
}

int showMenu() {
  int choice;
  bool _stop;
  _stop = false;
  do {
    printf("\n\n<--- MENU ---> \n");
    printf("1. List Contacts. \n");
    printf("2. Add User\n");
    printf("3. Delete user\n");
    printf("4. Send Messages\n");
    printf("5. Check Messages\n");
    printf("6. Close the client\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice <= 0 && choice > 6)
      _stop = true;
  } while (_stop);
  return choice;
}

void userChoiceOperations(int choice, int client_socket, char *userId) {
  switch (choice) {
  case 1:
    printf("\tYour Contacts:\n");
    getContacts(userId, client_socket);
    break;
  case 2:
    printf("\tAdding User:\n");
    addUser(client_socket);
    break;
  case 3:
    printf("\tDeleting User:\n");
    deleteUser(client_socket);
    break;
  case 4:
    printf("\tSending Messages:\n");
    sendMessages(client_socket);
    break;
  case 5:
    checkMessages(client_socket);
    printf("\tChecking Messages:\n");
    break;
  case 6:
    printf("\tClosing the client now...\n");
    break;
  defult:
    break;
  }
}
//// MENU
// 1. get contacts
void getContacts(char *userId, int client_socket) {
  char *command = "takeContacts:userId";
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
// 2. add user
void addUser(int client_socket) {
  // client function
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
// 3. delete user
void deleteUser(int client_socket) {
  // client function
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
void sendMessages(int client_socket) {
  // client function
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

// 5. check messages
void checkMessages(int client_socket) {
  // client function
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
