#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// Global variables 
int server_socket = -1;             // Server socket
pthread_t recv_thread;              // Reception thread
bool connection_active = false;     // Connection state
pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to protect connection

void process_incoming_message(const char *message);

// Initialize connection to server
bool init_connection(void) {
    struct sockaddr_in server_addr;
    
    pthread_mutex_lock(&conn_mutex);
    
    // Check if already connected
    if (connection_active) {
        pthread_mutex_unlock(&conn_mutex);
        return true;
    }
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error when creating socket");
        pthread_mutex_unlock(&conn_mutex);
        return false;
    }
    
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    // Connect to server
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed.");
        close(server_socket);
        server_socket = -1;
        pthread_mutex_unlock(&conn_mutex);
        return false;
    }
    
    connection_active = true;
    pthread_mutex_unlock(&conn_mutex);
    
    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
    return true;
}

// Close connection to server
void close_connection(void) {
    pthread_mutex_lock(&conn_mutex);
    
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    
    connection_active = false;
    pthread_mutex_unlock(&conn_mutex);
}

// Thread receives messages from server
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int consecutive_failures = 0;
    
    while (1) {
        // Check if the connection is active
        pthread_mutex_lock(&conn_mutex);
        bool is_active = connection_active;
        int socket_fd = server_socket;
        pthread_mutex_unlock(&conn_mutex);
        
        if (!is_active || socket_fd < 0) {
            // SIf disconnected, wait and check again
            usleep(100000); // 100ms
            consecutive_failures++;
            
            if (consecutive_failures > 50) { // After about 5 seconds
                break;
            }
            continue;
        }
        
        // Receive message
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            // Reception problem
            consecutive_failures++;
            
            if (consecutive_failures > 3) {
                // For consecutive failures, consider we are disconnected
                pthread_mutex_lock(&conn_mutex);
                connection_active = false;
                pthread_mutex_unlock(&conn_mutex);
                
                // Inform user
                process_incoming_message("Disconnected from server.");
                break;
            }
            
            // Wait a bit and try again
            usleep(100000); // 100ms
            continue;
        }
        
        // Reset failure count
        consecutive_failures = 0;
        
        // Add a character at the end of the chain
        buffer[bytes_received] = '\0';
        
        // Process the message received
        process_incoming_message(buffer);
    }
    
    return NULL;
}

// Start reception thread
void start_receive_thread(void) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    if (pthread_create(&recv_thread, &attr, receive_messages, NULL) != 0) {
        perror("Error while creating thread");
        close_connection();
    }
    
    pthread_attr_destroy(&attr);
}

// Send message to server
bool send_message_to_server(const char *message) {
    pthread_mutex_lock(&conn_mutex);
    
    if (!connection_active || server_socket < 0) {
        pthread_mutex_unlock(&conn_mutex);
        return false;
    }
    
    int socket_fd = server_socket;
    pthread_mutex_unlock(&conn_mutex);
    
    if (send(socket_fd, message, strlen(message), 0) < 0) {
        perror("Error sending the message.");
        
        pthread_mutex_lock(&conn_mutex);
        connection_active = false;
        pthread_mutex_unlock(&conn_mutex);
        
        return false;
    }
    
    return true;
}

// Check if connection is active
bool is_connection_active(void) {
    pthread_mutex_lock(&conn_mutex);
    bool active = connection_active;
    pthread_mutex_unlock(&conn_mutex);
    
    return active;
}