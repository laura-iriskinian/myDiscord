#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <pthread.h>   
#include "client.h"

// Variables
int server_socket;
pthread_t recv_thread;  

// Connection to server
bool init_connection(void) {
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect");
        return false;
    }

    return true;
}

// Close connection to server
void close_connection(void) {
    close(server_socket);
}

// Send a message to server
bool send_to_server(const char* message) {
    return send(server_socket, message, strlen(message), 0) > 0;
}

// Receive a message from server 
bool receive_from_server(char* buffer, int size) {
    int len = recv(server_socket, buffer, size - 1, 0);
    if (len <= 0) return false;
    buffer[len] = '\0';
    return true;
}

// Function called in interface.c
void send_message_to_server(const char *message) {
    if (!send_to_server(message)) {
        fprintf(stderr, "Erreur lors de l'envoi du message au serveur.\n");
    }
}

// reception thread
void* receive_thread(void* arg) {
    char buffer[1024];

    while (1) {
        if (!receive_from_server(buffer, sizeof(buffer))) {
            printf("Déconnexion du serveur.\n");
            break;
        }
        printf("Message reçu: %s\n", buffer);
    }

    close_connection();
    return NULL;
}

// Start reception thread 
void start_receive_thread(void) {
    if (pthread_create(&recv_thread, NULL, receive_thread, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
}