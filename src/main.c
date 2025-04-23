#include <gtk/gtk.h>
#include "gui.h"

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.MyDiscord", G_APPLICATION_DEFAULT_FLAGS);
    load_css();
    g_signal_connect(app, "activate", G_CALLBACK(show_choice_screen), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
