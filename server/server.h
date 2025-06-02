#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

// Structure to stock information of a connected user
typedef struct {
    int socket;                 
    int id;                     
    char username[50];          
    bool authenticated;         
    int current_channel_id;     
    pthread_t thread;           
} client_t;

// Main functions of the server
bool init_server(void);
void run_server(void);
void stop_server(void);

// Client management
void register_client(int client_socket);
void disconnect_client(int client_id);
void broadcast_message(int sender_id, const char* message, int channel_id);
void send_to_client(int client_id, const char* message);

// Authentication
bool authenticate_user(int client_id, const char* email, const char* password);
bool register_user(int client_id, const char* firstname, const char* lastname, 
                   const char* email, const char* password);

// Channel management
bool join_channel(int client_id, int channel_id);
bool create_channel(int client_id, const char* channel_name, bool is_private);
bool delete_channel(int client_id, int channel_id);

#endif 