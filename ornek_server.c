#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8082
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void store_message(int uid, char *msg);
void send_stored_messages(int sockfd, int uid);


typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int uid;
} client_t;

typedef struct {
    int uid;
    char message[BUFFER_SIZE];
} message_t;

message_t messages[MAX_CLIENTS][10]; // Simplified storage for messages (10 messages per client)
int message_count[MAX_CLIENTS] = {0};

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int nbytes;

    while ((nbytes = recv(cli->sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[nbytes] = '\0';

        if (strcmp(buffer, "CHECK_MESSAGES") == 0) {
            send_stored_messages(cli->sockfd, cli->uid);
        } else {
            int recipient_uid;
            char message[BUFFER_SIZE];
            sscanf(buffer, "%d:%[^\n]", &recipient_uid, message);
            store_message(recipient_uid, message);
            printf("User %d sent a message to user %d: %s\n", cli->uid, recipient_uid, message);
        }
    }

    printf("User %d has disconnected.\n", cli->uid);
    close(cli->sockfd);
    remove_client(cli->uid);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}

void store_message(int uid, char *msg) {
    if (message_count[uid] < 10) {
        strncpy(messages[uid][message_count[uid]].message, msg, BUFFER_SIZE);
        messages[uid][message_count[uid]].uid = uid;
        message_count[uid]++;
    } else {
        printf("Message storage full for user %d\n", uid);
    }
}

void send_stored_messages(int sockfd, int uid) {
    for (int i = 0; i < message_count[uid]; i++) {
        send(sockfd, messages[uid][i].message, strlen(messages[uid][i].message), 0);
        usleep(100000); // Small delay to ensure messages are sent separately
    }
    message_count[uid] = 0; // Clear messages after sending
    char *end_of_messages = "END_OF_MESSAGES";
    send(sockfd, end_of_messages, strlen(end_of_messages), 0);
}

int main() {
    int sockfd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) < 0) {
        perror("Socket listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d...\n", PORT);

    int uid = 0;
    while (1) {
        socklen_t clilen = sizeof(client_addr);
        new_sock = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);

        if (new_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = client_addr;
        cli->sockfd = new_sock;
        cli->uid = uid++;

        add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        printf("User %d connected.\n", cli->uid);
    }

    return 0;
}