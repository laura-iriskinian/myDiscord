#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
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
static void on_register_button_clicked(GtkButton *button, gpointer user_data);
static void on_send_message_clicked(GtkButton *button, gpointer user_data);

static GtkWidget* build_choice_screen(void);
static GtkWidget* build_signin_screen(void);
static GtkWidget* build_register_screen(void);
static GtkWidget* build_chat_screen(void);

// Functions to navigate between the different screens keeping the same window
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

// Functions to create the different screens within the window
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
    
    // Stock the fields references
    g_object_set_data(G_OBJECT(signin_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(signin_button), "entry_password", entry_password);
    
    // Connect click manager
    g_signal_connect(signin_button, "clicked", G_CALLBACK(on_signin_button_clicked), NULL);
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

    // stock the references in the fields
    g_object_set_data(G_OBJECT(register_button), "entry_name", entry_name);
    g_object_set_data(G_OBJECT(register_button), "entry_first_name", entry_first_name);
    g_object_set_data(G_OBJECT(register_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(register_button), "entry_password", entry_password);

    // Connect click manager
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), NULL);
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

// Function to validate email address
bool validate_email(const char *email) {
    // Check if email contains @
    const char *at_sign = strchr(email, '@');
    if (!at_sign) return false;
    
    // Check if there are letters before and after @
    if (at_sign == email || *(at_sign + 1) == '\0') return false;
    
    // Check if there is a dot after @
    const char *dot_after_at = strchr(at_sign, '.');
    if (!dot_after_at) return false;
    
    // Check if there is text after dot
    if (*(dot_after_at + 1) == '\0') return false;
    
    return true;
}

// Function to validate password at least 10 characters
bool validate_password(const char *password) {
    if (strlen(password) < 10) return false;
    
    bool has_lowercase = false;
    bool has_uppercase = false;
    bool has_digit = false;
    bool has_special = false;
    
    for (size_t i = 0; i < strlen(password); i++) {
        char c = password[i];
        if (islower(c)) has_lowercase = true;
        else if (isupper(c)) has_uppercase = true;
        else if (isdigit(c)) has_digit = true;
        else has_special = true;
    }
    
    return has_lowercase && has_uppercase && has_digit && has_special;
}

void show_error_dialog(const char *message) {
    // Use general error message to avoid UTF-8 problems
    const char *safe_message = "An error occurred during authentication.\nPlease try again with valid credentials.";
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Error",
                                                  GTK_WINDOW(window),
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  "OK",
                                                  GTK_RESPONSE_ACCEPT,
                                                  NULL);
    
    // Créer le contenu
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(safe_message);
    gtk_box_append(GTK_BOX(content_area), label);
    
    // Connecter le signal de réponse
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    
    // Afficher le dialog
    gtk_widget_set_visible(dialog, TRUE);
}

// Function to validate client registering
static void on_register_button_clicked(GtkButton *button, gpointer user_data) {
    // Get entries
    GtkEntry *entry_name = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_name"));
    GtkEntry *entry_first_name = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_first_name"));
    GtkEntry *entry_mail = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_mail"));
    GtkEntry *entry_password = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_password"));
    
    const char *name = gtk_editable_get_text(GTK_EDITABLE(entry_name));
    const char *first_name = gtk_editable_get_text(GTK_EDITABLE(entry_first_name));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(name) == 0 || strlen(first_name) == 0 || strlen(email) == 0 || strlen(password) == 0) {
        // Afficher un message d'erreur en utilisant notre fonction helper
        show_error_dialog("Please complete every field");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Invalid email address");
        return;
    }
    
    // Validate password
    if (!validate_password(password)) {
        show_error_dialog("Password must be at least 10 characters, "
                         "one upper case and one lower case letter, "
                         "one digit and one special character");
        return;
    }
    
    // Prepare and send command to server
    char command[BUFFER_SIZE];
    sprintf(command, "/register %s %s %s %s", first_name, name, email, password);
    send_message_to_server(command);
    
    // Move on to chat screen
    show_chat_screen();
}

// Function to validate sign-in 
static void on_signin_button_clicked(GtkButton *button, gpointer user_data) {
    // Get entries
    GtkEntry *entry_mail = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_mail"));
    GtkEntry *entry_password = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_password"));
    
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(email) == 0 || strlen(password) == 0) {
        show_error_dialog("Please complete every field");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Invalid email address");
        return;
    }
    
    // Prepare and send command to server
    char command[BUFFER_SIZE];
    sprintf(command, "/login %s %s", email, password);
    send_message_to_server(command);
    
    // Wait for server response with a short delay
    g_timeout_add(500, (GSourceFunc)check_auth_response, button);
}

// Check authentication response 
static gboolean check_auth_response(GtkButton *button) {
    char response[256] = {0};
    bool success = get_auth_status(response, sizeof(response));
    
    // Si une réponse a été reçue
    if (auth_response_received()) {
        if (success) {
            // Authentication successfull, move to chat screen
            g_print("Auth success, moving to chat screen\n");
            show_chat_screen();
        } else {
            // Display error message
            g_print("Auth failed, showing error dialog\n");
            show_error_dialog(response);
        }
        return G_SOURCE_REMOVE; 
    }
    
    // Si aucune réponse reçue, continuer à vérifier
    return G_SOURCE_CONTINUE;
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

// Send messages
static void on_send_message_clicked(GtkButton *button, gpointer user_data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);

    char *message = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (message && strlen(message) > 0) {
        printf("[DEBUG] Message to send : %s\n", message);

        if (message[0] == '/') {
            // it is a command, send directly
            send_message_to_server(message);
        } else {
            // It is a normal message, add to local chat
            GtkTextBuffer *history_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_history));
            GtkTextIter history_end;
            gtk_text_buffer_get_end_iter(history_buffer, &history_end);
            
            // Format message in history
            gchar *formatted = g_strdup_printf("Me: %s\n", message);
            gtk_text_buffer_insert(history_buffer, &history_end, formatted, -1);
            g_free(formatted);

            // Send the message
            send_message_to_server(message);
        }

        // Clear text zone after sending
        gtk_text_buffer_set_text(buffer, "", -1);
    }

    g_free(message);
}