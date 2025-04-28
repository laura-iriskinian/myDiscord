#include <gtk/gtk.h>
#include <string.h>
#include "interface.h"
#include "client.h"

GtkWidget *window = NULL;
GtkWidget *stack = NULL;
GtkWidget *chat_text_view = NULL; 
GtkWidget *chat_history = NULL;  

// Forward declarations
void show_choice_screen(void);
void show_signin_form(void);
void show_register_form(void);
void show_chat_screen(void);

static GtkWidget* build_choice_screen(void);
static GtkWidget* build_signin_screen(void);
static GtkWidget* build_register_screen(void);
static GtkWidget* build_chat_screen(void);

// Functions to navigate between the different screens keeping teh same window

void show_choice_screen(void) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "choice_screen");
}

void show_signin_form(void) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "signin_screen");
}

void show_register_form(void) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "register_screen");
}

void show_chat_screen(void) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "chat_screen");
}

// Send messages

static void on_send_message_clicked(GtkButton *button, gpointer user_data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);

    char *message = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (message && strlen(message) > 0) {
        printf("[DEBUG] Message à envoyer : %s\n", message);

        // Add message to history
        GtkTextBuffer *history_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_history));
        GtkTextIter history_end;
        gtk_text_buffer_get_end_iter(history_buffer, &history_end);
        
        // Format of message in history
        gchar *formatted = g_strdup_printf("Moi: %s\n", message);
        gtk_text_buffer_insert(history_buffer, &history_end, formatted, -1);
        g_free(formatted);

        // send message
        send_message_to_server(message);

        // Clear text area after sending
        gtk_text_buffer_set_text(buffer, "", -1);
    }

    g_free(message);
}

// Functiosn to create the different screens within the window

static GtkWidget* build_choice_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *signin_button = gtk_button_new_with_label("Sign In");
    GtkWidget *register_button = gtk_button_new_with_label("Register");

    g_signal_connect(signin_button, "clicked", G_CALLBACK(show_signin_form), NULL);
    g_signal_connect(register_button, "clicked", G_CALLBACK(show_register_form), NULL);

    gtk_box_append(GTK_BOX(box), signin_button);
    gtk_box_append(GTK_BOX(box), register_button);

    return box;
}

static GtkWidget* build_signin_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("Sign In");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *signin_button = gtk_button_new_with_label("Sign in");
    // For now connection button leads to chat screen
    g_signal_connect(signin_button, "clicked", G_CALLBACK(show_chat_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), signin_button);

    GtkWidget *back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), back_button);
    
    gtk_box_append(GTK_BOX(box), button_box);

    return box;
}

static GtkWidget* build_register_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("Register");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Name");
    gtk_box_append(GTK_BOX(box), entry_name);
    
    GtkWidget *entry_first_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_first_name), "First name");
    gtk_box_append(GTK_BOX(box), entry_first_name);
    
    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *register_button = gtk_button_new_with_label("Register");
    // For now sign-in button leads to chat screen
    g_signal_connect(register_button, "clicked", G_CALLBACK(show_chat_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), register_button);

    GtkWidget *back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), back_button);
    
    gtk_box_append(GTK_BOX(box), button_box);

    return box;
}

static GtkWidget* build_chat_screen(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_hexpand(vbox, TRUE);
    gtk_widget_set_vexpand(vbox, TRUE);

    // Chat messages history
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll_window, TRUE);
    
    chat_history = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_history), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chat_history), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_history), GTK_WRAP_WORD_CHAR);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), chat_history);
    gtk_box_append(GTK_BOX(vbox), scroll_window);

    // Message typing area
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *msg_scroll = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(msg_scroll, TRUE);
    
    chat_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_text_view), GTK_WRAP_WORD_CHAR);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(msg_scroll), chat_text_view);

    GtkWidget *send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message_clicked), NULL);

    gtk_box_append(GTK_BOX(hbox), msg_scroll);
    gtk_box_append(GTK_BOX(hbox), send_button);

    gtk_box_append(GTK_BOX(vbox), hbox);

    return vbox;
}

// Main window 

void build_main_window(GtkApplication *app) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MyDiscord Client");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_widget_set_hexpand(stack, TRUE);
    gtk_widget_set_vexpand(stack, TRUE);

    // Link to CSS style sheet
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
    g_object_unref(css_provider);

    // Add the different screens to the stack
    gtk_stack_add_named(GTK_STACK(stack), build_choice_screen(), "choice_screen");
    gtk_stack_add_named(GTK_STACK(stack), build_signin_screen(), "signin_screen");
    gtk_stack_add_named(GTK_STACK(stack), build_register_screen(), "register_screen");
    gtk_stack_add_named(GTK_STACK(stack), build_chat_screen(), "chat_screen");

    gtk_window_set_child(GTK_WINDOW(window), stack);

    gtk_widget_show(window);
}