#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

// General setup information
#define SERVER_IP "127.0.0.1"  
#define SERVER_PORT 8080       

// Connection functions
bool init_connection(void);
void close_connection(void);
void start_receive_thread(void);

// Functions to receive/send
bool send_to_server(const char* message);
bool receive_from_server(char* buffer, int size);
void send_message_to_server(const char *message);

#endif 