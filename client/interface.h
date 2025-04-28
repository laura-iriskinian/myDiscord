#ifndef INTERFACE_H
#define INTERFACE_H

#include <gtk/gtk.h>

// Functiosn to build the interface
void build_main_window(GtkApplication *app);

// Functions to navigate between the different screens
void show_choice_screen(void);
void show_signin_form(void);
void show_register_form(void);
void show_chat_screen(void);

#endif 