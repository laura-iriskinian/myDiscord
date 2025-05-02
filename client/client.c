#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <pthread.h>   
#include "client.h"
#include "interface.h"

// Variables
int server_socket = -1;
pthread_t recv_thread;  
static char server_response[1024];
static bool auth_response_flag = false;
static bool auth_success = false;
static bool connection_active = false;

// Connection to server
bool is_connection_active(void) {
    return connection_active;  // Où connection_active est une variable globale dans client.c
}
bool init_connection(void) {
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(server_socket);
        server_socket = -1;
        return false;
    }

    connection_active = true;
    return true;
}

// Close connection to server
void close_connection(void) {
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    connection_active = false;
}

// Send a message to server
bool send_to_server(const char* message) {
    if (server_socket < 0 || !connection_active) {
        fprintf(stderr, "Impossible d'envoyer un message: pas de connexion active au serveur.\n");
        return false;
    }
    
    return send(server_socket, message, strlen(message), 0) > 0;
}

// Receive a message from server 
bool receive_from_server(char* buffer, int size) {
    if (server_socket < 0 || !connection_active) {
        return false;
    }
    
    int len = recv(server_socket, buffer, size - 1, 0);
    if (len <= 0) {
        connection_active = false;
        return false;
    }
    
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

    while (connection_active) {
        if (!receive_from_server(buffer, sizeof(buffer))) {
            printf("Disconnection from server.\n");
            break;
        }
        
        printf("Message received: %s\n", buffer);
        
        // Process authentication responses
        if (strstr(buffer, "Authentication") != NULL || 
            strstr(buffer, "successfull") != NULL ||
            strstr(buffer, "Authentification") != NULL ||
            strstr(buffer, "Error:") != NULL || 
            strstr(buffer, "Incorrect") != NULL) {
            handle_auth_response(buffer);
        }
        
        // Process other messages (like chat messages)
        process_incoming_message(buffer);
    }

    connection_active = false;
    return NULL;
}

// Start reception thread 
void start_receive_thread(void) {
    if (pthread_create(&recv_thread, NULL, receive_thread, NULL) != 0) {
        perror("Failed to create receive thread");
        connection_active = false;
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
    
    auth_response_flag = true;
}

// Function to get authentication state
bool get_auth_status(char *response_message, size_t size) {
    // If the response has not been received yet, return false
    if (!auth_response_flag) {
        return false;
    }
    
    // Copy message if response requested
    if (response_message != NULL && size > 0) {
        strncpy(response_message, server_response, size - 1);
        response_message[size - 1] = '\0';
    }
    
    // Save state before clearing for next auth
    bool success = auth_success;
    
    // Reset for next authentication
    auth_response_flag = false;
    
    return success;
}