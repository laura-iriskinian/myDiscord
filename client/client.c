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
static char server_response[1024];
static bool auth_response_received_flag = false;
static bool auth_success = false;

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
    // Reset auth flag when sending new login command
    if (strstr(message, "/login") != NULL) {
        auth_response_received_flag = false;
        auth_success = false;
    }
    
    if (!send_to_server(message)) {
        fprintf(stderr, "Erreur lors de l'envoi du message au serveur.\n");
    }
}

// reception thread
void* receive_thread(void* arg) {
    char buffer[1024];

    while (1) {
        if (!receive_from_server(buffer, sizeof(buffer))) {
            printf("Disconnection from server.\n");
            break;
        }
        
        printf("Message received: %s\n", buffer);
        
        // Traiter les réponses d'authentification
        if (strstr(buffer, "Authentification successfull") != NULL || 
            strstr(buffer, "Error: Incorrect") != NULL ||
            strstr(buffer, "has connected") != NULL) {
            handle_auth_response(buffer);
        }
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

// Function to handle authentication responses
void handle_auth_response(const char *message) {
    strncpy(server_response, message, sizeof(server_response) - 1);
    server_response[sizeof(server_response) - 1] = '\0';
    printf("DEBUG: Processing auth response: %s\n", message);
    
    // Check if one of the messages is an authentication confirmation
    if (strstr(message, "successfull") != NULL) {
        auth_success = true;
        printf("DEBUG: Authentication success detected\n");
    } 
    // If an error message is detected, authentication failed
    else if (strstr(message, "Error:") != NULL || strstr(message, "Incorrect") != NULL) {
        auth_success = false;
        printf("DEBUG: Authentication failure detected\n");
    }
    // If the message indicates successful connection
    else if (strstr(message, "has connected") != NULL) {
        auth_success = true;
        printf("DEBUG: Connection message detected, assuming success\n");
    }
    
    auth_response_received_flag = true;
}

// Function to check if auth response was received
bool auth_response_received(void) {
    return auth_response_received_flag;
}

// Function to get authentication state
bool get_auth_status(char *response_message, size_t size) {
    // Copy message if response requested
    if (response_message != NULL && size > 0) {
        strncpy(response_message, server_response, size - 1);
        response_message[size - 1] = '\0';
    }
    
    return auth_success;
}