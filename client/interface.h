#ifndef INTERFACE_H
#define INTERFACE_H

#include <gtk/gtk.h>

static void on_register_button_clicked(GtkButton *button, gpointer user_data);
static gboolean check_auth_response(GtkButton *button);
static void on_signin_button_clicked(GtkButton *button, gpointer user_data);
static void on_send_message_clicked(GtkButton *button, gpointer user_data);

// Define buffer size for the messages
#define BUFFER_SIZE 1024 

// Functions to build the interface
void build_main_window(GtkApplication *app);

// Functions to navigate between the different screens
void show_choice_screen(void);
void show_signin_form(void);
void show_register_form(void);
void show_chat_screen(void);

// Functions to validate the entries
bool validate_email(const char *email);
bool validate_password(const char *password);

#endif 