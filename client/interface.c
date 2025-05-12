#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h> // used for islower, isupper, isdigit in password verification
#include "interface.h"
#include "client.h"

// Structure to represent a channel in the interface
typedef struct {
    int id;             // channel ID 
    char name[50];      // Channel name
    bool is_private;    // is channel private
} UIChannel;

// Structure to represent a message in the interface
typedef struct {
    int id;             // message ID
    char sender[50];    // Name of sender
    char content[1024]; // message content
    char timestamp[20]; // timestamp
} UIMessage;

// Structures for app database
typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *chat_text_view;
    GtkWidget *chat_history;
    GtkWidget *main_chat_area;
    int current_channel_id;
} AppData;

// global variable for app data
static AppData app_data;

// Forward declarations
static void show_choice_screen(void);
static void show_signin_form(void);
static void show_register_form(void);
static void show_chat_screen(void);
static void on_register_button_clicked(GtkButton *button, gpointer user_data);
static void on_signin_button_clicked(GtkButton *button, gpointer user_data);
static void on_send_message_clicked(GtkButton *button, gpointer user_data);
static void on_theme_button_clicked(GtkButton *button, gpointer user_data);
static void on_channel_clicked(GtkButton *button, gpointer id_ptr);
static void update_channel_messages(int channel_id);
static gboolean check_auth_response(GtkButton *button);
static void show_error_dialog(const char *message);

// Declaration of is_connected first to prevent conflict
static bool is_connected(void);

// Screen building
static GtkWidget* build_choice_screen(void);
static GtkWidget* build_signin_screen(void);
static GtkWidget* build_register_screen(void);
static GtkWidget* build_chat_screen(void);

bool has_connection_error = false;
char connection_error_message[256] = "";

// Function to check periodically the state of the connection
static gboolean check_connection_status(gpointer data) {
    if (!is_connected()) {
        if (!has_connection_error) {
            has_connection_error = true;
            strncpy(connection_error_message, "Connection to server lost.", sizeof(connection_error_message));
            
            // Display error message
            show_error_dialog("Connection to server lost.");
            
            // Return to main screen
            show_choice_screen();
        }
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

// Function to check if the client is connected to the server
static bool is_connected(void) {
    return is_connection_active();
}

// Functions to navigate between the different screens keeping the same window
static void show_choice_screen(void) {
    if (app_data.stack != NULL) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.stack), "choice_screen");
    }
}

static void show_signin_form(void) {
    if (app_data.stack != NULL) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.stack), "signin_screen");
    }
}

static void show_register_form(void) {
    if (app_data.stack != NULL) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.stack), "register_screen");
    }
}

static void show_chat_screen(void) {
    if (app_data.stack != NULL) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.stack), "chat_screen");
        
        // Initialize current channel(main channel by default)
        app_data.current_channel_id = 1;
        
        // Update messages
        update_channel_messages(app_data.current_channel_id);
    }
}

// Manage change of interface theme
static void on_theme_button_clicked(GtkButton *button, gpointer user_data) {
    const char *theme = gtk_widget_get_name(GTK_WIDGET(button));

    if (app_data.window != NULL) {
        // Remove all themes
        gtk_widget_remove_css_class(app_data.window, "theme-dark");
        gtk_widget_remove_css_class(app_data.window, "theme-light");
        gtk_widget_remove_css_class(app_data.window, "theme-violet");

        // Apply new theme
        gtk_widget_add_css_class(app_data.window, theme);
    }
}

// Build main window 
void build_main_window(GtkApplication *app) {
    // Initialize AppData structure
    memset(&app_data, 0, sizeof(AppData));
    
    // Create main window
    app_data.window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data.window), "MyDiscord Client");
    gtk_window_set_default_size(GTK_WINDOW(app_data.window), 1200, 800);
    
    // Apply theme by default
    gtk_widget_add_css_class(app_data.window, "theme-dark");

    // Create the stack for the different views
    app_data.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app_data.stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_widget_set_hexpand(app_data.stack, TRUE);
    gtk_widget_set_vexpand(app_data.stack, TRUE);

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
    gtk_stack_add_named(GTK_STACK(app_data.stack), build_choice_screen(), "choice_screen");
    gtk_stack_add_named(GTK_STACK(app_data.stack), build_signin_screen(), "signin_screen");
    gtk_stack_add_named(GTK_STACK(app_data.stack), build_register_screen(), "register_screen");
    gtk_stack_add_named(GTK_STACK(app_data.stack), build_chat_screen(), "chat_screen");

    // Set the window content
    gtk_window_set_child(GTK_WINDOW(app_data.window), app_data.stack);

    // Present the window
    gtk_window_present(GTK_WINDOW(app_data.window));
}

// Functions to build the different screens in the same window
static GtkWidget* build_choice_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("Welcome to my Discord");
    gtk_widget_add_css_class(label, "welcome-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *signin_button = gtk_button_new_with_label("Connect");
    GtkWidget *register_button = gtk_button_new_with_label("Register");
    
    gtk_widget_add_css_class(signin_button, "my-button");
    gtk_widget_add_css_class(register_button, "my-button");

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

    GtkWidget *label = gtk_label_new("Connection");
    gtk_widget_add_css_class(label, "form-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *signin_button = gtk_button_new_with_label("Connection");
    gtk_widget_add_css_class(signin_button, "my-button");
    
    // Stock the fields references
    g_object_set_data(G_OBJECT(signin_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(signin_button), "entry_password", entry_password);
    
    // Connect click manager
    g_signal_connect(signin_button, "clicked", G_CALLBACK(on_signin_button_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), signin_button);

    GtkWidget *back_button = gtk_button_new_with_label("Back");
    gtk_widget_add_css_class(back_button, "my-button");
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), back_button);
    
    gtk_box_append(GTK_BOX(box), button_box);

    return box;
}

static GtkWidget* build_register_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("Registration");
    gtk_widget_add_css_class(label, "form-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Name");
    gtk_widget_add_css_class(entry_name, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_name);
    
    GtkWidget *entry_first_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_first_name), "First name");
    gtk_widget_add_css_class(entry_first_name, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_first_name);
    
    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *register_button = gtk_button_new_with_label("Register");
    gtk_widget_add_css_class(register_button, "my-button");

    // stock the references in the fields
    g_object_set_data(G_OBJECT(register_button), "entry_name", entry_name);
    g_object_set_data(G_OBJECT(register_button), "entry_first_name", entry_first_name);
    g_object_set_data(G_OBJECT(register_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(register_button), "entry_password", entry_password);

    // Connect click manager
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), register_button);

    GtkWidget *back_button = gtk_button_new_with_label("Back");
    gtk_widget_add_css_class(back_button, "my-button");
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), back_button);
    
    gtk_box_append(GTK_BOX(box), button_box);

    return box;
}

static GtkWidget* build_chat_screen(void) {
    // Creation main horizontal layout
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // 1. Server panel (to the left)
    GtkWidget *servers_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(servers_panel, 70, -1);
    gtk_widget_add_css_class(servers_panel, "servers");
    
    // For now we only have one server
    GtkWidget *server_btn = gtk_button_new_with_label("MD");
    gtk_widget_add_css_class(server_btn, "server-button");
    gtk_box_append(GTK_BOX(servers_panel), server_btn);
    
    // 2. Channel panel(next to server panel)
    GtkWidget *channels_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(channels_panel, 200, -1);
    gtk_widget_add_css_class(channels_panel, "conversations");
    
    // Show a few test channels
    const char *channel_names[] = {"General", "Notice board", "Community help", "General-private", "Discussions"};
    int channel_ids[] = {1, 2, 3, 4, 5};
    
    for (int i = 0; i < 5; i++) {
        GtkWidget *channel_btn = gtk_button_new_with_label(channel_names[i]);
        gtk_widget_add_css_class(channel_btn, "conversation-button");
        
        // Alloocate and stock channel ID to use in callback
        int *id_ptr = g_new(int, 1);
        *id_ptr = channel_ids[i];
        g_signal_connect(channel_btn, "clicked", G_CALLBACK(on_channel_clicked), id_ptr);
        
        gtk_box_append(GTK_BOX(channels_panel), channel_btn);
    }
    
    // 3. Chat panel 
    GtkWidget *chat_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(chat_panel, "chat");
    gtk_widget_set_hexpand(chat_panel, TRUE);
    gtk_widget_set_vexpand(chat_panel, TRUE);
    
    // Message area (scrollable)
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll_window, TRUE);
    
    app_data.main_chat_area = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_data.main_chat_area), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app_data.main_chat_area), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_data.main_chat_area), GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(app_data.main_chat_area, "chat-area");
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), app_data.main_chat_area);
    gtk_box_append(GTK_BOX(chat_panel), scroll_window);
    
    // Text entry area
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *msg_scroll = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(msg_scroll, TRUE);
    
    app_data.chat_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_data.chat_text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(app_data.chat_text_view, "chat-entry");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(msg_scroll), app_data.chat_text_view);
    
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_widget_add_css_class(send_button, "my-button");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message_clicked), NULL);
    
    gtk_box_append(GTK_BOX(input_box), msg_scroll);
    gtk_box_append(GTK_BOX(input_box), send_button);
    
    gtk_box_append(GTK_BOX(chat_panel), input_box);
    
    // 4. Profile pannel (on the right)
    GtkWidget *profile_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(profile_panel, 140, -1);
    gtk_widget_add_css_class(profile_panel, "profile");
    
    GtkWidget *profile_label = gtk_label_new("Profile");
    gtk_widget_add_css_class(profile_label, "profile-label");
    gtk_box_append(GTK_BOX(profile_panel), profile_label);
    
    GtkWidget *theme_label = gtk_label_new("Select a theme :");
    gtk_widget_add_css_class(theme_label, "theme-label");
    gtk_box_append(GTK_BOX(profile_panel), theme_label);
    
    // Theme buttons
    const char *themes[] = {"theme-dark", "theme-light", "theme-violet"};
    const char *labels[] = {"Dark theme", "Light theme", "Purple theme"};
    
    for (int i = 0; i < 3; i++) {
        GtkWidget *theme_btn = gtk_button_new_with_label(labels[i]);
        gtk_widget_add_css_class(theme_btn, "theme-button");
        gtk_widget_set_name(theme_btn, themes[i]);
        g_signal_connect(theme_btn, "clicked", G_CALLBACK(on_theme_button_clicked), NULL);
        gtk_box_append(GTK_BOX(profile_panel), theme_btn);
    }
    
    // Disconnection button
    GtkWidget *logout_button = gtk_button_new_with_label("Disconnect");
    gtk_widget_add_css_class(logout_button, "my-button");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(profile_panel), logout_button);
    
    // Assemble all panels
    gtk_box_append(GTK_BOX(main_box), servers_panel);
    gtk_box_append(GTK_BOX(main_box), channels_panel);
    gtk_box_append(GTK_BOX(main_box), chat_panel);
    gtk_box_append(GTK_BOX(main_box), profile_panel);
    
    return main_box;
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

static void show_error_dialog(const char *message) {
    // Use general error message to avoid UTF-8 problems
    const char *safe_message = message ? message : 
                              "An error occured druing authentication.\nPlease try again with valid credentials.";
    
    // Create a modern dialog box
    if (app_data.window != NULL) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", safe_message);
        gtk_alert_dialog_show(dialog, GTK_WINDOW(app_data.window)); 
        g_object_unref(dialog);
    } else {
        g_print("Error: %s\n", safe_message);
    }
}

// Manage change of channel
static void on_channel_clicked(GtkButton *button, gpointer id_ptr) {
    if (id_ptr != NULL) {
        int channel_id = *(int*)id_ptr;
        
        // Update current channel
        app_data.current_channel_id = channel_id;
        
        // Command server to join the channel
        char command[BUFFER_SIZE];
        sprintf(command, "/join %d", channel_id);
        send_message_to_server(command);
        
        // Update the displayed messages
        update_channel_messages(channel_id);
    }
}

// Update the channel messages
static void update_channel_messages(int channel_id) {
    if (app_data.main_chat_area != NULL) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.main_chat_area));
        
        // Delete current content
        gtk_text_buffer_set_text(buffer, "", -1);
        
        // While waiting for server messages, loading message displayed
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, "Loading messages...\n", -1);
    }
}

// Function to validate client registering
static void on_register_button_clicked(GtkButton *button, gpointer user_data) {
    // Get entries
    GtkEntry *entry_name = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_name"));
    GtkEntry *entry_first_name = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_first_name"));
    GtkEntry *entry_mail = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_mail"));
    GtkEntry *entry_password = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_password"));
    
    if (entry_name == NULL || entry_first_name == NULL || entry_mail == NULL || entry_password == NULL) {
        show_error_dialog("Internal error in the app.");
        return;
    }
    
    const char *name = gtk_editable_get_text(GTK_EDITABLE(entry_name));
    const char *first_name = gtk_editable_get_text(GTK_EDITABLE(entry_first_name));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(name) == 0 || strlen(first_name) == 0 || strlen(email) == 0 || strlen(password) == 0) {
        show_error_dialog("Please complete all fields");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Invalid email address");
        return;
    }
    
    // Validate password
    if (!validate_password(password)) {
        show_error_dialog("The password must contain at least 10 characters, "
                         "one upper case letter, one lower case letter, "
                         "one digit and a special character");
        return;
    }
    
    // Prepare and send command to server
    char command[BUFFER_SIZE];
    sprintf(command, "/register %s %s %s %s", first_name, name, email, password);
    send_message_to_server(command);
    
    // Wait for server response with a short delay
    g_timeout_add(500, (GSourceFunc)check_auth_response, button);
}

// Function to validate sign-in 
static void on_signin_button_clicked(GtkButton *button, gpointer user_data) {
    // Get entries
    GtkEntry *entry_mail = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_mail"));
    GtkEntry *entry_password = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry_password"));
    
    if (entry_mail == NULL || entry_password == NULL) {
        show_error_dialog("Internal error in the app.");
        return;
    }
    
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(email) == 0 || strlen(password) == 0) {
        show_error_dialog("Please complete all fields");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Invalid email");
        return;
    }
    
     // Notify user that authentication is in process
     gtk_button_set_label(button, "Connection in process...");
     gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

    // Prepare and send command to server
    char command[BUFFER_SIZE];
    sprintf(command, "/login %s %s", email, password);
    send_message_to_server(command);
    
    // Wait for server response with a short delay
    g_timeout_add(500, (GSourceFunc)check_auth_response, button);
}

// Check authentication response 
static gboolean check_auth_response(GtkButton *button) {
    char response[256];
    bool success = get_auth_status(response, sizeof(response));
    
    if (strlen(response) > 0) {
        if (success) {
            // Authentication successful, move to chat screen
            show_chat_screen();
        } else {
            // Display error message
            show_error_dialog(response);
        }
        return G_SOURCE_REMOVE; 
    }
    
    // If no response received, try again later
    return G_SOURCE_CONTINUE;
}

// Function to add a message to chat
void add_message_to_chat(const char *message) {
    if (app_data.main_chat_area == NULL) return;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.main_chat_area));
    GtkTextIter iter;
    
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, message, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    
    // Scroll to the bottom to see the most recent message
    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app_data.main_chat_area), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(buffer, mark);
}

// Send messages
static void on_send_message_clicked(GtkButton *button, gpointer user_data) {
    if (app_data.chat_text_view == NULL) return;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.chat_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);

    char *message = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (message && strlen(message) > 0) {
        if (message[0] == '/') {
            // It is a command, send directly
            send_message_to_server(message);
        } else {
            // It is a normal message, add to local chat
            if (app_data.main_chat_area != NULL) {
                GtkTextBuffer *history_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.main_chat_area));
                GtkTextIter history_end;
                gtk_text_buffer_get_end_iter(history_buffer, &history_end);
                
                // Format message in history
                gchar *formatted = g_strdup_printf("Moi: %s\n", message);
                gtk_text_buffer_insert(history_buffer, &history_end, formatted, -1);
                g_free(formatted);
            }

            // Send the message
            send_message_to_server(message);
        }

        // Clear text zone after sending
        gtk_text_buffer_set_text(buffer, "", -1);
    }

    g_free(message);
}

// Function to handle incoming messages from server
void process_incoming_message(const char *message) {
    // When in chat screen, display message
    if (app_data.stack != NULL && 
        gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)) &&
        strcmp(gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)), "chat_screen") == 0) {
        
        // Add the message to the chat area
        add_message_to_chat(message);
    }
}

// Function to create a new channel
void create_new_channel(const char *name, bool is_private) {
    char command[BUFFER_SIZE];
    if (is_private) {
        sprintf(command, "/create %s private", name);
    } else {
        sprintf(command, "/create %s", name);
    }
    send_message_to_server(command);
}

// Function to delete a channel
void delete_channel(int channel_id) {
    char command[BUFFER_SIZE];
    sprintf(command, "/delete %d", channel_id);
    send_message_to_server(command);
}

// Function to update user interface
void update_ui() {
    // If on the chat screen, update the channels and messages
    if (app_data.stack != NULL && 
        gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)) &&
        strcmp(gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)), "chat_screen") == 0) {
        
        // Update all message of current channel
        update_channel_messages(app_data.current_channel_id);
    }
}
