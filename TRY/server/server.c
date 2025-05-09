#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <libpq-fe.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

// Structure to store info of a connected client
typedef struct {
    int socket;                 // Client socket
    int id;                     // unique ID of client
    int user_id;                // user ID in database
    char username[50];          // user name
    bool authenticated;         // Authenticated or not
    int current_channel;        // Actual channel (1 = main)
    pthread_t thread;           // Thread managing the client
} Client;

// Global variables 
Client *clients[MAX_CLIENTS];                       // Table of connected clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // To protect access to the table
int server_socket;                                  // Server socket
bool server_running = true;                         // State of server
PGconn *db_connection = NULL;                       // Connection to database

// Function prototypes
void *handle_client(void *arg);
void broadcast_message(int sender_id, const char *message, int channel);
void send_to_client(int client_id, const char *message);
void disconnect_client(int client_id);
bool init_database(void);
void close_database(void);
PGresult *execute_query(const char *query);
bool create_tables_if_not_exists(void);
bool verify_login(const char *email, const char *password, int *user_id, char *username);
bool register_user(const char *firstname, const char *lastname, const char *email, const char *password);
char *hash_password(const char *password);
void send_channel_history(int client_id, int channel_id);
bool save_message(int user_id, int channel_id, const char *content);

// Initialize database 
bool init_database(void) {
    // Connection to database
    const char *db_params = "host=localhost port=5432 dbname=mydiscord user=postgres password=Ecole2024!";
    
    // Attempt connection
    db_connection = PQconnectdb(db_params);
    
    // Check connection state
    if (PQstatus(db_connection) != CONNECTION_OK) {
        fprintf(stderr, "Error when connecting to database: %s\n", PQerrorMessage(db_connection));
        PQfinish(db_connection);
        db_connection = NULL;
        return false;
    }
    
    printf("Connected to database successfull.\n");
    
    // Create tables if don't exist
    return create_tables_if_not_exists();
}

// Close connection to the database
void close_database(void) {
    if (db_connection) {
        PQfinish(db_connection);
        db_connection = NULL;
    }
}

// Run SQL request
PGresult *execute_query(const char *query) {
    if (!db_connection) {
        fprintf(stderr, "No connection to the database.\n");
        return NULL;
    }
    
    PGresult *result = PQexec(db_connection, query);
    
    if (!result) {
        fprintf(stderr, "Error executing the request: %s\n", PQerrorMessage(db_connection));
        return NULL;
    }
    
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        fprintf(stderr, "Request failed: %s\n", PQerrorMessage(db_connection));
        fprintf(stderr, "Request: %s\n", query);
        PQclear(result);
        return NULL;
    }
    
    return result;
}

// Create tables if they don't exist
bool create_tables_if_not_exists(void) {
    // Users table
    const char *users_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "    id SERIAL PRIMARY KEY,"
        "    username VARCHAR(50) NOT NULL,"
        "    firstname VARCHAR(50) NOT NULL,"
        "    lastname VARCHAR(50) NOT NULL,"
        "    email VARCHAR(100) UNIQUE NOT NULL,"
        "    password VARCHAR(100) NOT NULL"
        ");";
    
    // Channel table
    const char *channels_table = 
        "CREATE TABLE IF NOT EXISTS channels ("
        "    id SERIAL PRIMARY KEY,"
        "    name VARCHAR(50) NOT NULL,"
        "    is_private BOOLEAN DEFAULT FALSE"
        ");";
    
    // Messages table
    const char *messages_table = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "    id SERIAL PRIMARY KEY,"
        "    user_id INTEGER REFERENCES users(id),"
        "    channel_id INTEGER REFERENCES channels(id),"
        "    content TEXT NOT NULL,"
        "    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    // Execute requests
    PGresult *result;
    
    result = execute_query(users_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(channels_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(messages_table);
    if (!result) return false;
    PQclear(result);
    
    // Check if main channel already exists
    result = execute_query("SELECT id FROM channels WHERE id=1");
    if (!result) return false;
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        
        // Create main channel
        result = execute_query(
            "INSERT INTO channels (id, name, is_private) "
            "VALUES (1, 'Général', FALSE)"
        );
        
        if (!result) return false;
        PQclear(result);
    } else {
        PQclear(result);
    }
    
    return true;
}

// Simple hashing of password
char *hash_password(const char *password) {
    if (!password) return NULL;
    
    // Secret key for the XOR
    const char *key = "MyDiscordSecretKey";
    size_t keylen = strlen(key);
    size_t passlen = strlen(password);
    
    // Allocation of memory for the hash
    char *hashed = (char*)malloc(passlen * 2 + 1);
    if (!hashed) return NULL;
    
    // Simple hash with XOR
    for (size_t i = 0; i < passlen; i++) {
        unsigned char c = password[i] ^ key[i % keylen];
        sprintf(hashed + i*2, "%02x", c);
    }
    
    hashed[passlen * 2] = '\0';
    return hashed;
}

// Check connection identification
bool verify_login(const char *email, const char *password, int *user_id, char *username) {
    char query[256];
    sprintf(query, "SELECT id, username, password FROM users WHERE email='%s'", email);
    
    PGresult *result = execute_query(query);
    if (!result) return false;
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Get user information
    *user_id = atoi(PQgetvalue(result, 0, 0));
    strcpy(username, PQgetvalue(result, 0, 1));
    char stored_password[100];
    strcpy(stored_password, PQgetvalue(result, 0, 2));
    
    PQclear(result);
    
    // Check the paswword
    char *hashed = hash_password(password);
    if (!hashed) return false;
    
    bool match = (strcmp(hashed, stored_password) == 0);
    free(hashed);
    
    return match;
}

// Save a new user
bool register_user(const char *firstname, const char *lastname, const char *email, const char *password) {
    // Check if email already exists
    char query[256];
    sprintf(query, "SELECT id FROM users WHERE email='%s'", email);
    
    PGresult *result = execute_query(query);
    if (!result) return false;
    
    if (PQntuples(result) > 0) {
        PQclear(result);
        return false; // Email already used
    }
    
    PQclear(result);
    
    // Create hash for the password
    char *hashed = hash_password(password);
    if (!hashed) return false;
    
    // Generate the username
    char username[100];
    sprintf(username, "%s %s", firstname, lastname);
    
    // Insert new user
    sprintf(query, 
            "INSERT INTO users (username, firstname, lastname, email, password) "
            "VALUES ('%s', '%s', '%s', '%s', '%s')",
            username, firstname, lastname, email, hashed);
    
    free(hashed);
    
    result = execute_query(query);
    if (!result) return false;
    
    PQclear(result);
    return true;
}

// Send message history of the channel
void send_channel_history(int client_id, int channel_id) {
    char query[300];
    sprintf(query, 
            "SELECT m.content, u.username, m.timestamp "
            "FROM messages m "
            "JOIN users u ON m.user_id = u.id "
            "WHERE m.channel_id = %d "
            "ORDER BY m.timestamp ASC "
            "LIMIT 50",
            channel_id);
    
    PGresult *result = execute_query(query);
    if (!result) return;
    
    // Send header
    send_to_client(client_id, "--- History of messages ---");
    
    // Go through results
    int count = PQntuples(result);
    for (int i = 0; i < count; i++) {
        char message[BUFFER_SIZE];
        sprintf(message, "[%s] %s: %s", 
                PQgetvalue(result, i, 2),  // timestamp
                PQgetvalue(result, i, 1),  // username
                PQgetvalue(result, i, 0)); // content
        
        send_to_client(client_id, message);
    }
    
    send_to_client(client_id, "--- End of history ---");
    
    PQclear(result);
}

// Save message in database
bool save_message(int user_id, int channel_id, const char *content) {
    char query[1500];
    sprintf(query, 
            "INSERT INTO messages (user_id, channel_id, content) "
            "VALUES (%d, %d, '%s')",
            user_id, channel_id, content);
    
    PGresult *result = execute_query(query);
    if (!result) return false;
    
    PQclear(result);
    return true;
}

// Initialize server
bool init_server(void) {
    struct sockaddr_in server_addr;
    
    // Initialize database
    if (!init_database()) {
        fprintf(stderr, "Error initializing database.\n");
        return false;
    }
    
    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating the socket.");
        close_database();
        return false;
    }
    
    // Configuration to reuse address
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configuration of server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Link socket to address
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error when binding");
        close(server_socket);
        close_database();
        return false;
    }
    
    // Put socket in listening mode
    if (listen(server_socket, 10) < 0) {
        perror("Error during listening.");
        close(server_socket);
        close_database();
        return false;
    }
    
    // Initialize client tables
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    
    printf("Server started on port %d\n", SERVER_PORT);
    return true;
}

// Main function to run server
void run_server(void) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;
    
    while (server_running) {
        // Wait for a new connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            if (server_running) {
                perror("Error when accepting the connection.");
            }
            continue;
        }
        
        // Find a free place in the table
        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == NULL) {
                break;
            }
        }
        
        if (i == MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            printf("Server full, connection declined.\n");
            close(client_socket);
            continue;
        }
        
        // Create and initialize client
        clients[i] = malloc(sizeof(Client));
        if (!clients[i]) {
            pthread_mutex_unlock(&clients_mutex);
            perror("Memory allocation error.");
            close(client_socket);
            continue;
        }
        
        clients[i]->socket = client_socket;
        clients[i]->id = i;
        clients[i]->user_id = -1;  // Not authenticated yet
        strcpy(clients[i]->username, "Anonymous");
        clients[i]->authenticated = false;
        clients[i]->current_channel = 1;  // Main channel by default
        
        printf("New client connected with ID %d\n", i);
        
        // Create a thread to generate this client
        if (pthread_create(&clients[i]->thread, NULL, handle_client, (void*)(long)i) != 0) {
            perror("Error when creating client thread");
            free(clients[i]);
            clients[i] = NULL;
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket);
            continue;
        }
        
        pthread_mutex_unlock(&clients_mutex);
        
        // Display welcome message
        send_to_client(i, "Welcome to MyDiscord!");
    }
}

// Manage connected client
void *handle_client(void *arg) {
    int client_id = (int)(long)arg;
    char buffer[BUFFER_SIZE];
    int read_size;
    
    // Get client socket
    pthread_mutex_lock(&clients_mutex);
    if (clients[client_id] == NULL) {
        pthread_mutex_unlock(&clients_mutex);
        return NULL;
    }
    int client_socket = clients[client_id]->socket;
    pthread_mutex_unlock(&clients_mutex);
    
    // Main loop to receive messages
    while ((read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        // Check that client still exists
        pthread_mutex_lock(&clients_mutex);
        if (clients[client_id] == NULL) {
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        pthread_mutex_unlock(&clients_mutex);
        
        printf("Message received from client %d: %s\n", client_id, buffer);
        
        // Deal with commands
        if (buffer[0] == '/') {
            char command[20];
            char args[BUFFER_SIZE];
            
            sscanf(buffer, "/%s %[^\n]", command, args);
            
            if (strcmp(command, "login") == 0) {
                char email[100], password[100];
                if (sscanf(args, "%s %s", email, password) == 2) {
                    int user_id;
                    char username[50];
                    
                    if (verify_login(email, password, &user_id, username)) {
                        pthread_mutex_lock(&clients_mutex);
                        if (clients[client_id]) {
                            clients[client_id]->user_id = user_id;
                            strcpy(clients[client_id]->username, username);
                            clients[client_id]->authenticated = true;
                            pthread_mutex_unlock(&clients_mutex);
                            
                            char response[100];
                            sprintf(response, "Connection successfull as %s", username);
                            send_to_client(client_id, response);
                            
                            // Announce connection
                            char announce[100];
                            sprintf(announce, "%s has joined the chat", username);
                            broadcast_message(-1, announce, 1);
                            
                            // Send message history
                            send_channel_history(client_id, 1);
                        } else {
                            pthread_mutex_unlock(&clients_mutex);
                        }
                    } else {
                        send_to_client(client_id, "Error: incorrect email or password.");
                    }
                } else {
                    send_to_client(client_id, "Usage: /login email password");
                }
            }
            else if (strcmp(command, "register") == 0) {
                char firstname[50], lastname[50], email[100], password[100];
                if (sscanf(args, "%s %s %s %s", firstname, lastname, email, password) == 4) {
                    if (register_user(firstname, lastname, email, password)) {
                        send_to_client(client_id, "Registration successfull. Use /login to connect.");
                    } else {
                        send_to_client(client_id, "Error: Registration failed. Email might already be in use.");
                    }
                } else {
                    send_to_client(client_id, "Usage: /register first name name email password");
                }
            }
            else if (strcmp(command, "p") == 0 || strcmp(command, "private") == 0) {
                // Private message: /p recipient message
                char target[50];
                char message[BUFFER_SIZE];
                
                if (sscanf(args, "%s %[^\n]", target, message) == 2) {
                    // Search for recipient
                    int target_id = -1;
                    
                    pthread_mutex_lock(&clients_mutex);
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i] && strcmp(clients[i]->username, target) == 0) {
                            target_id = i;
                            break;
                        }
                    }
                    
                    if (target_id != -1 && clients[client_id] && clients[client_id]->authenticated) {
                        char private_msg[BUFFER_SIZE];
                        sprintf(private_msg, "[MP de %s] %s", clients[client_id]->username, message);
                        pthread_mutex_unlock(&clients_mutex);
                        
                        send_to_client(target_id, private_msg);
                        send_to_client(client_id, "Private message sent");
                    } else {
                        pthread_mutex_unlock(&clients_mutex);
                        send_to_client(client_id, "User cannot be found");
                    }
                } else {
                    send_to_client(client_id, "Usage: /p message recipient");
                }
            }
            else {
                send_to_client(client_id, "Unknown command");
            }
        }
        else {
            // Normal message to broadcast
            pthread_mutex_lock(&clients_mutex);
            if (clients[client_id] && clients[client_id]->authenticated) {
                int user_id = clients[client_id]->user_id;
                int channel = clients[client_id]->current_channel;
                pthread_mutex_unlock(&clients_mutex);
                
                // Save message in the database
                save_message(user_id, channel, buffer);
                
                // Boradcast message
                broadcast_message(client_id, buffer, channel);
            } else {
                pthread_mutex_unlock(&clients_mutex);
                send_to_client(client_id, "You must first connect with /login");
            }
        }
    }
    
    // Disconnected client
    printf("Client %d disconnected\n", client_id);
    disconnect_client(client_id);
    
    return NULL;
}

// Send a message to all clients on a channel
void broadcast_message(int sender_id, const char *message, int channel) {
    pthread_mutex_lock(&clients_mutex);
    
    char formatted_message[BUFFER_SIZE];
    
    if (sender_id == -1) {
        // System message
        sprintf(formatted_message, "[Serveur] %s", message);
    } else if (clients[sender_id]) {
        // User message
        sprintf(formatted_message, "[%s] %s", clients[sender_id]->username, message);
    } else {
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    
    // Send to all the clients in the same channel
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->authenticated && clients[i]->current_channel == channel) {
            send(clients[i]->socket, formatted_message, strlen(formatted_message), 0);
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Send a message to specific client
void send_to_client(int client_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id]) {
        send(clients[client_id]->socket, message, strlen(message), 0);
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Disconnect a client
void disconnect_client(int client_id) {
    pthread_mutex_lock(&clients_mutex);
    
    if (client_id < 0 || client_id >= MAX_CLIENTS || !clients[client_id]) {
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    
    // Announce disconnection
    if (clients[client_id]->authenticated) {
        char message[100];
        sprintf(message, "%s has disconnected", clients[client_id]->username);
        
        pthread_mutex_unlock(&clients_mutex);
        broadcast_message(-1, message, 1);
        pthread_mutex_lock(&clients_mutex);
    }
    
    // Close socket and free resources
    close(clients[client_id]->socket);
    free(clients[client_id]);
    clients[client_id] = NULL;
    
    pthread_mutex_unlock(&clients_mutex);
}

// Stop server
void stop_server(void) {
    server_running = false;
    
    // Close server socket
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
    
    // Close connection to database
    close_database();
    
    printf("Server has stopped.\n");
}