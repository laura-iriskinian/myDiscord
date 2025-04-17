#include <gtk/gtk.h>
#include "gui.h"

void on_app_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Connexion - MyDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);

    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "Nom");

    GtkWidget *entry_prenom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_prenom), "Prénom");

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");

    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);

    GtkWidget *button = gtk_button_new_with_label("Se connecter");

    gtk_grid_attach(GTK_GRID(grid), entry_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_mail, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);

    gtk_window_set_child(GTK_WINDOW(window), grid);
    gtk_window_present(GTK_WINDOW(window));
}
