#include <gtk/gtk.h>
#include "gui.h"

// Create window interface
void on_app_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Sign in - MyDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    // Create widgets on grid
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    // Entry widgets creation
    GtkWidget *entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Name");

    GtkWidget *entry_first_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_first_name), "First name");

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");

    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE); // Hide password when typed

    // Sign-in button creation
    GtkWidget *button = gtk_button_new_with_label("Sign in");
    gtk_widget_set_margin_top(button,30);

    // Place widgets in window
    gtk_grid_attach(GTK_GRID(grid), entry_name, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_first_name, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_mail, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 1, 1);

    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Link to CSS style sheet
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_path(provider, "log_style.css");
    g_object_unref(provider);

    // Create class for CSS style sheet
    gtk_widget_add_css_class(entry_name, "my-entry");
    gtk_widget_add_css_class(entry_first_name, "my-entry");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_widget_add_css_class(button, "my-button");

    gtk_window_present(GTK_WINDOW(window));
}
