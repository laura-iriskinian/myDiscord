#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

void show_choice_screen(GtkApplication *app);
void show_signin_form(GtkWindow *window);
void show_register_form(GtkWindow *window);
void load_css(void);
void go_back_to_choice_screen(GtkWindow *window);
void show_choice_screen(GtkApplication *app);

#endif
