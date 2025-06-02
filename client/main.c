#include <gtk/gtk.h>
#include "interface.h"
#include "client.h"

static void activate(GtkApplication *app, gpointer user_data) {
    build_main_window(app);
}

int main(int argc, char* argv[]) {
    // store temp files in a separate tmp folder
    setenv("XDG_RUNTIME_DIR", "/tmp", 1);
    // initialize GTK
    GtkApplication *app;
    int status;

    // Initialize connection to server
    if (!init_connection()) {
        fprintf(stderr, "Impossible to connect to server. The app will continue in disconnected mode.\n");
    } else {
        start_receive_thread();
    }

    // Start the GTK interface app
    app = gtk_application_new("com.example.mydiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Clean
    close_connection();
    g_object_unref(app);
    return status;
}