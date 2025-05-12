#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "database.h"
#include "channel.h"
#include "user.h"
#include "message.h"

// Global variables for the server
static int server_socket;
static client_t *clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool server_running = false;

// Functions to parse commands
void parse_command(int client_id, char *buffer);

// Thread to manage a client
void *handle_client(void *arg);

// Initialize the server
bool init_server(void) {
    struct sockaddr_in server_addr;
    
    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating the socket");
        return false;
    }
    
    // Configuration of socket to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_socket);
        return false;
    }
    
    // Configuration of server's address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Link socket to address
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error while binding");
        close(server_socket);
        return false;
    }
    
    // Socket listening
    if (listen(server_socket, 10) < 0) {
        perror("Error while listening");
        close(server_socket);
        return false;
    }
    
    // Initialize the database
    if (!init_database()) {
        fprintf(stderr, "Error when initializing the database\n");
        close(server_socket);
        return false;
    }

    // Create tables if they don't exist
    if (!create_tables_if_not_exists()) {
        fprintf(stderr, "Error when creating tables\n");
        close_database();
        close(server_socket);
        return false;
}
    
    // Initialize client table
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    
    server_running = true;
    printf("Server started on port %d\n", SERVER_PORT);
    return true;
}

// Run the server
void run_server(void) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;
    
    while (server_running) {
        // Wait for a connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            if (server_running) {
                perror("Error when accepting the connection");
            }
            continue;
        }
        
        // Register the client
        register_client(client_socket);
        
        printf("New connection of %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
}

// Stop server
void stop_server(void) {
    server_running = false;
    
    // Close the server socket to release accept()
    close(server_socket);
    
    // Close all client connections
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            close(clients[i]->socket);
            pthread_cancel(clients[i]->thread);
            free(clients[i]);
            clients[i] = NULL;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    // Close the database
    close_database();
    
    printf("Server stopped\n");
}

// Register a new client
void register_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    
    // Find a free slot
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            break;
        }
    }
    
    if (i == MAX_CLIENTS) {
        pthread_mutex_unlock(&clients_mutex);
        fprintf(stderr, "Sever full, connection denied\n");
        close(client_socket);
        return;
    }
    
    // Create and initialize the client
    client_t *client = (client_t*)malloc(sizeof(client_t));
    if (!client) {
        pthread_mutex_unlock(&clients_mutex);
        perror("Error allocating memory");
        close(client_socket);
        return;
    }
    
    client->socket = client_socket;
    client->id = i;
    strcpy(client->username, "Anonymous");
    client->authenticated = false;
    client->current_channel_id = 1; // Main channel by default
    
    clients[i] = client;
    pthread_mutex_unlock(&clients_mutex);
    
    //  Create a thread to handle the client
    if (pthread_create(&client->thread, NULL, handle_client, (void*)(long)i) != 0) {
        perror("Error while creating the thread");
        pthread_mutex_lock(&clients_mutex);
        free(clients[i]);
        clients[i] = NULL;
        pthread_mutex_unlock(&clients_mutex);
        close(client_socket);
        return;
    }
    
    // Send a welcome menu
    char welcome[100];
    sprintf(welcome, "Welcome on MyDiscord! Please sign yourself in.");
    send_to_client(i, welcome);
}

// Dicsonnect a client
void disconnect_client(int client_id) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id]) {
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    
    // Announce disconnection
    if (clients[client_id]->authenticated) {
        char message[100];
        sprintf(message, "%s is disconnected", clients[client_id]->username);
        
        // Free the mutex during the diffusion
        pthread_mutex_unlock(&clients_mutex);
        broadcast_message(-1, message, clients[client_id]->current_channel_id);
        pthread_mutex_lock(&clients_mutex);
    }
    
    // Close the socket and free resources
    printf("Client %d disconnected\n", client_id);
    close(clients[client_id]->socket);
    free(clients[client_id]);
    clients[client_id] = NULL;
    
    pthread_mutex_unlock(&clients_mutex);
}

// Send a message to all the members of a channel
void broadcast_message(int sender_id, const char* message, int channel_id) {
    pthread_mutex_lock(&clients_mutex);
    
    char formatted_message[BUFFER_SIZE];
    
    // Format of the message depending on who sends it
    if (sender_id == -1) {
        // System message 
        snprintf(formatted_message, sizeof(formatted_message), "[Server] %s", message);
    } else if (clients[sender_id]) {
        // User message
        snprintf(formatted_message, sizeof(formatted_message), "[%s] %s", 
                 clients[sender_id]->username, message);
        
        // Save the message in the database
        save_message(sender_id, channel_id, message);
    }
    
    // Send to all clients in the same channel
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->authenticated && 
            clients[i]->current_channel_id == channel_id) {
            
            send(clients[i]->socket, formatted_message, strlen(formatted_message), 0);
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Send a message to a specific client
void send_to_client(int client_id, const char* message) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id]) {
        send(clients[client_id]->socket, message, strlen(message), 0);
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Authenticate a user
bool authenticate_user(int client_id, const char* email, const char* password) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id]) {
        pthread_mutex_unlock(&clients_mutex);
        return false;
    }
    
    // Check all users in the database
    user_t user;
    bool success = verify_login(email, password, &user);
    
    if (success) {
        // Mark as authenticated and copy informatio
        clients[client_id]->authenticated = true;
        strcpy(clients[client_id]->username, user.username);
        
        // Announce the connection
        char message[100];
        sprintf(message, "%s has connected", clients[client_id]->username);
        
        pthread_mutex_unlock(&clients_mutex);
        broadcast_message(-1, message, clients[client_id]->current_channel_id);
        
        return true;
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return false;
}

// Register a new user
bool register_user(int client_id, const char* firstname, const char* lastname, 
                   const char* email, const char* password) {
    
    // Create the user in the database
    user_t new_user;
    strncpy(new_user.firstname, firstname, sizeof(new_user.firstname) - 1);
    strncpy(new_user.lastname, lastname, sizeof(new_user.lastname) - 1);
    strncpy(new_user.email, email, sizeof(new_user.email) - 1);
    
    // Generate username (first name + name)
    snprintf(new_user.username, sizeof(new_user.username), "%s %s", firstname, lastname);
    
    // Save in teh database
    if (create_user(&new_user, password)) {
        // Authenticate directly
        return authenticate_user(client_id, email, password);
    }
    
    return false;
}

// Join a channel
bool join_channel(int client_id, int channel_id) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id] || 
        !clients[client_id]->authenticated) {
        pthread_mutex_unlock(&clients_mutex);
        return false;
    }
    
    // Check if the channel exists and if user has rights
    if (!channel_exists(channel_id) || !can_access_channel(client_id, channel_id)) {
        pthread_mutex_unlock(&clients_mutex);
        return false;
    }
    
    // Leave current channel
    int old_channel = clients[client_id]->current_channel_id;
    char leave_message[100];
    sprintf(leave_message, "%s has left the channel", clients[client_id]->username);
    
    // Change channel
    clients[client_id]->current_channel_id = channel_id;
    
    // Welcome message to a channel
    char join_message[100];
    sprintf(join_message, "%s has joined the channel", clients[client_id]->username);
    
    pthread_mutex_unlock(&clients_mutex);
    
    // Announce departure and arrival
    broadcast_message(-1, leave_message, old_channel);
    broadcast_message(-1, join_message, channel_id);
    
    // Send history of channel messages 
    send_channel_history(client_id, channel_id);
    
    return true;
}

// Create new channel
bool create_channel(int client_id, const char* channel_name, bool is_private) {
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id] || 
        !clients[client_id]->authenticated) {
        return false;
    }
    
    // Create new channel in the database
    int channel_id = add_channel(channel_name, is_private, client_id);
    if (channel_id <= 0) {
        return false;
    }
    
    // Announce creation of the new channel
    char message[150];
    sprintf(message,"The new channel '%s' was created by %s (ID: %d)", 
            channel_name, clients[client_id]->username, channel_id);
    broadcast_message(-1, message, 1); // Announce in the main channel
    
    return true;
}

// Delete a channel
bool delete_channel(int client_id, int channel_id) {
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id] || 
        !clients[client_id]->authenticated) {
        return false;
    }
    
    // Check the rights (must be admin or channel creator)
    if (!is_channel_admin(client_id, channel_id)) {
        return false;
    }
    
    // Do not delete main channel
    if (channel_id == 1) {
        return false;
    }
    
    // Get the name before deletion
    char channel_name[50];
    get_channel_name(channel_id, channel_name);
    
    // Move all users towards the main channel
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->authenticated && 
            clients[i]->current_channel_id == channel_id) {
            clients[i]->current_channel_id = 1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    // Delete channel
    if (remove_channel(channel_id)) {
        // Announce deletion
        char message[150];
        sprintf(message, "The channel '%s' was deleted by %s", 
                channel_name, clients[client_id]->username);
        broadcast_message(-1, message, 1);
        return true;
    }
    
    return false;
}

// Parse commands received by client
void parse_command(int client_id, char *buffer) {
    // Commands supported:
    // /login email password - Connection
    // /register firstname lastname email password - registration
    // /join channel_id - Join a channel
    // /create channel_name [private] - Create a channel
    // /delete channel_id - Delete a channel
    // /msg message - Send a message (by default if no command)
    
    // Check if it is a command (starts with /)
    if (buffer[0] == '/') {
        char command[20];
        char args[BUFFER_SIZE - 20];
        
        // Extract the command
        sscanf(buffer, "/%s %[^\n]", command, args);
        
        if (strcmp(command, "login") == 0) {
            char email[50], password[50];
            if (sscanf(args, "%s %s", email, password) == 2) {
                if (authenticate_user(client_id, email, password)) {
                    send_to_client(client_id, "Authentication successfull");
                } else {
                    send_to_client(client_id, "Error: Incorrect email or password");
                }
            } else {
                send_to_client(client_id, "Usage: /login email password");
            }
        }
        else if (strcmp(command, "register") == 0) {
            char firstname[50], lastname[50], email[50], password[50];
            if (sscanf(args, "%s %s %s %s", firstname, lastname, email, password) == 4) {
                if (register_user(client_id, firstname, lastname, email, password)) {
                    send_to_client(client_id, "Registration successfull");
                } else {
                    send_to_client(client_id, "Error: Registration failed");
                }
            } else {
                send_to_client(client_id, "Usage: /register firstname lastname email password");
            }
        }
        else if (strcmp(command, "join") == 0) {
            int channel_id;
            if (sscanf(args, "%d", &channel_id) == 1) {
                if (join_channel(client_id, channel_id)) {
                    char response[100];
                    sprintf(response, "You have joined the channel %d", channel_id);
                    send_to_client(client_id, response);
                } else {
                    send_to_client(client_id, "Error: Impossible to join this channel");
                }
            } else {
                send_to_client(client_id, "Usage: /join channel_id");
            }
        }
        else if (strcmp(command, "create") == 0) {
            char channel_name[50];
            bool is_private = false;
            
            // Check if a channel is private
            if (strstr(args, "private")) {
                is_private = true;
                sscanf(args, "%s private", channel_name);
            } else {
                sscanf(args, "%s", channel_name);
            }
            
            if (strlen(channel_name) > 0) {
                if (create_channel(client_id, channel_name, is_private)) {
                    send_to_client(client_id, "Channel created successfully");
                } else {
                    send_to_client(client_id, "Error: Impossible to create the channel");
                }
            } else {
                send_to_client(client_id, "Usage: /create channel_name [private]");
            }
        }
        else if (strcmp(command, "delete") == 0) {
            int channel_id;
            if (sscanf(args, "%d", &channel_id) == 1) {
                if (delete_channel(client_id, channel_id)) {
                    send_to_client(client_id, "Channel deleted successfully");
                } else {
                    send_to_client(client_id, "Error: Impossible to delete this channel");
                }
            } else {
                send_to_client(client_id, "Usage: /delete channel_id");
            }
        }
        else {
            send_to_client(client_id, "Commande unknown");
        }
    }
    else {
        // If it's a normal message
        pthread_mutex_lock(&clients_mutex);
        
        if (!clients[client_id] || !clients[client_id]->authenticated) {
            pthread_mutex_unlock(&clients_mutex);
            send_to_client(client_id, "Error: You must be signed in to send messages");
            return;
        }
        
        int channel_id = clients[client_id]->current_channel_id;
        pthread_mutex_unlock(&clients_mutex);
        
        broadcast_message(client_id, buffer, channel_id);
    }
}

// Thread manages client's connection
void *handle_client(void *arg) {
    int client_id = (int)(long)arg;
    char buffer[BUFFER_SIZE];
    int read_size;
    
    while ((read_size = recv(clients[client_id]->socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        parse_command(client_id, buffer);
    }
    
    // Client disconnected
    if (read_size == 0) {
        printf("Client %d is disconnected\n", client_id);
    } else if (read_size == -1) {
        perror("Receiving error");
    }
    
    disconnect_client(client_id);
    return NULL;
}