#include <gtk/gtk.h>
#include "gui.h"

void show_register_form(GtkWindow *window);
void show_signin_form(GtkWindow *window);
void show_choice_screen(GtkApplication *app);
void load_css(void);
void show_choice_screen(GtkApplication *app);
void go_back_to_choice_screen(GtkWindow *window);

// Create window interface
void show_choice_screen(GtkApplication *app) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Welcome to MyDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    
    load_css();
    go_back_to_choice_screen(GTK_WINDOW(window));

    gtk_window_present(GTK_WINDOW(window));
}

void show_signin_form(GtkWindow *window) {
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);

    GtkWidget *btn_signin = gtk_button_new_with_label("Sign in");
    gtk_widget_set_margin_top(btn_signin, 30);
    GtkWidget *btn_back = gtk_button_new_with_label("Back");
    gtk_widget_set_margin_top(btn_back, 30);

    gtk_grid_attach(GTK_GRID(grid), entry_mail, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_signin, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_back, 0, 4, 1, 1);

    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_widget_add_css_class(btn_signin, "my-button");
    gtk_widget_add_css_class(btn_back, "my-button");

    gtk_window_set_child(window, grid);

    g_signal_connect_swapped(btn_back, "clicked", G_CALLBACK(go_back_to_choice_screen), window);
}

void show_register_form(GtkWindow *window) {
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Name");
    GtkWidget *entry_first_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_first_name), "First name");
    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);

    GtkWidget *btn_register = gtk_button_new_with_label("Register");
    gtk_widget_set_margin_top(btn_register, 30);
    GtkWidget *btn_back = gtk_button_new_with_label("Back");
    gtk_widget_set_margin_top(btn_back, 10);

    gtk_grid_attach(GTK_GRID(grid), entry_name, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_first_name, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_mail, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_register, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_back, 0, 5, 1, 1);

    gtk_widget_add_css_class(entry_name, "my-entry");
    gtk_widget_add_css_class(entry_first_name, "my-entry");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_widget_add_css_class(btn_register, "my-button");
    gtk_widget_add_css_class(btn_back, "my-button");

    gtk_window_set_child(window, grid);

    g_signal_connect_swapped(btn_back, "clicked", G_CALLBACK(go_back_to_choice_screen), window);
}

void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "log_style.css");

    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

void go_back_to_choice_screen(GtkWindow *window) {
    // Créer les widgets pour l'écran de choix
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *btn_signin = gtk_button_new_with_label("Sign in");
    GtkWidget *btn_register = gtk_button_new_with_label("Register");

    gtk_widget_add_css_class(btn_signin, "my-button");
    gtk_widget_add_css_class(btn_register, "my-button");

    gtk_box_append(GTK_BOX(box), btn_signin);
    gtk_box_append(GTK_BOX(box), btn_register);
    
    // Remplacer le contenu de la fenêtre
    gtk_window_set_child(window, box);

    // Connexions aux callbacks
    g_signal_connect_swapped(btn_signin, "clicked", G_CALLBACK(show_signin_form), window);
    g_signal_connect_swapped(btn_register, "clicked", G_CALLBACK(show_register_form), window);
    
    // Définir le titre de la fenêtre
    gtk_window_set_title(window, "Welcome - MyDiscord");
}
