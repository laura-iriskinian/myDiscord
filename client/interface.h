#ifndef INTERFACE_H
#define INTERFACE_H

#include <gtk/gtk.h>
#include <stdbool.h>

// Define buffer size for the messages
#define BUFFER_SIZE 1024 

// Functions to build the interface
void build_main_window(GtkApplication *app);

// Functions to process messages
void add_message_to_chat(const char *message);
void process_incoming_message(const char *message);

// Functions to manipulate channels
void create_new_channel(const char *name, bool is_private);
void delete_channel(int channel_id);

// Function to update the UI
void update_ui(void);

// Functions to validate the entries
bool validate_email(const char *email);
bool validate_password(const char *password);

#endif