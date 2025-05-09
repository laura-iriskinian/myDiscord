#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

// Prototypes for client functions
extern bool init_connection(void);
extern void close_connection(void);
extern void start_receive_thread(void);
extern bool send_message_to_server(const char *message);
extern bool is_connection_active(void);

// Global variables for user interface 
GtkWidget *window;                   // Main window
GtkWidget *text_view;                // Display messages
GtkTextBuffer *buffer;               // Buffer to stock messages
GtkWidget *entry;                    // Entry field for messages
GtkWidget *login_email;              // Enter email (connection)
GtkWidget *login_password;           // Enter password (connection)
GtkWidget *register_firstname;       // Enter first name (registration)
GtkWidget *register_lastname;        // Enter the name (registration)
GtkWidget *register_email;           // Enter email (registration)
GtkWidget *register_password;        // Enter password (registration)
GtkWidget *choice_box;               // Box for selection screen
GtkWidget *login_box;                // Box for connection screen
GtkWidget *register_box;             // Box for registration screen
GtkWidget *chat_box;                 // Box for chat screen

// Prototypes for interface functions
void show_choice_screen();
void show_login_screen();
void show_register_screen();
void show_chat_screen();

// Structure to go from message to main thread GTK
typedef struct {
    char message[BUFFER_SIZE];
} ThreadMessage;

// Function executed in the main thread GTK
static gboolean update_text_view_from_thread(gpointer user_data) {
    ThreadMessage *msg_data = (ThreadMessage *)user_data;
    
    // Add message to buffer
    gtk_text_buffer_insert_at_cursor(buffer, msg_data->message, -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    
    // Scroll to bottom
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(buffer, mark);
    
    // Free memory
    free(msg_data);
    
    // Function not to be recalled
    return FALSE;
}

// Thread-safe version of process_incoming_message
void process_incoming_message(const char *message) {
    if (!message) return;
    
    // Allocate structure to save message
    ThreadMessage *msg_data = malloc(sizeof(ThreadMessage));
    if (!msg_data) return;
    
    // Copy message in the structure
    strncpy(msg_data->message, message, BUFFER_SIZE - 1);
    msg_data->message[BUFFER_SIZE - 1] = '\0';
    
    // Program execution of update_text_view_from_thread in the main GTK thread
    g_idle_add(update_text_view_from_thread, msg_data);
}

// Function to display an error message
void show_error_dialog(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function called when click on "send"
void on_send_clicked(GtkButton *button, gpointer data) {
    // Get message typed
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
    
    // If message is not empty, send it
    if (text && strlen(text) > 0) {
        send_message_to_server(text);
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // clear field
    }
}

// Function called when click on "Connect" in the connection sc reen
void on_login_submit_clicked(GtkButton *button, gpointer data) {
    const char *email = gtk_entry_get_text(GTK_ENTRY(login_email));
    const char *password = gtk_entry_get_text(GTK_ENTRY(login_password));
    
    if (email && password && strlen(email) > 0 && strlen(password) > 0) {
        // Send connection request
        char login_cmd[256];
        sprintf(login_cmd, "/login %s %s", email, password);
        send_message_to_server(login_cmd);
        
        // Move on to chat screen (if server does not reply connection succeeded)
        gtk_widget_hide(login_box);
        gtk_widget_show(chat_box);
    } else {
        // Display error message
        show_error_dialog("Please complete all fields");
    }
}

// Function called when click on "Register" in registration screen
void on_register_submit_clicked(GtkButton *button, gpointer data) {
    const char *firstname = gtk_entry_get_text(GTK_ENTRY(register_firstname));
    const char *lastname = gtk_entry_get_text(GTK_ENTRY(register_lastname));
    const char *email = gtk_entry_get_text(GTK_ENTRY(register_email));
    const char *password = gtk_entry_get_text(GTK_ENTRY(register_password));
    
    if (firstname && lastname && email && password && 
        strlen(firstname) > 0 && strlen(lastname) > 0 && 
        strlen(email) > 0 && strlen(password) > 0) {
        
        // Check email format (simple)
        if (strchr(email, '@') == NULL || strchr(email, '.') == NULL) {
            show_error_dialog("Invalid email format.");
            return;
        }
        
        // Check length of password
        if (strlen(password) < 10) {
            show_error_dialog("The password must contain at least 10 characters");
            return;
        }
        
        // Send registration request
        char register_cmd[512];
        sprintf(register_cmd, "/register %s %s %s %s", firstname, lastname, email, password);
        send_message_to_server(register_cmd);
        
        // Return to connection screen
        show_login_screen();
    } else {
        // Display error message
        show_error_dialog("Please complete all fields");
    }
}

// Function called when click on "Connect" in selection screen
void on_choice_login_clicked(GtkButton *button, gpointer data) {
    show_login_screen();
}

// Function called when click on "Register" in selection screen
void on_choice_register_clicked(GtkButton *button, gpointer data) {
    show_register_screen();
}

// Function called when click on "Back" in selection screen
void on_login_back_clicked(GtkButton *button, gpointer data) {
    show_choice_screen();
}

// Function called when click on "Back" in registration screen
void on_register_back_clicked(GtkButton *button, gpointer data) {
    show_choice_screen();
}

// Function called when click on Enter in text field
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Return) {
        on_send_clicked(NULL, NULL);
        return TRUE;
    }
    return FALSE;
}

// Function called when close window
void on_window_destroy(GtkWidget *widget, gpointer data) {
    close_connection();
    gtk_main_quit();
}

// Function to display selection screen
void show_choice_screen() {
    gtk_widget_hide(login_box);
    gtk_widget_hide(register_box);
    gtk_widget_hide(chat_box);
    gtk_widget_show(choice_box);
}

// Function to display connection screen
void show_login_screen() {
    gtk_widget_hide(choice_box);
    gtk_widget_hide(register_box);
    gtk_widget_hide(chat_box);
    gtk_widget_show(login_box);
}

// Function to display registration screen
void show_register_screen() {
    gtk_widget_hide(choice_box);
    gtk_widget_hide(login_box);
    gtk_widget_hide(chat_box);
    gtk_widget_show(register_box);
}

// Function to display chat screen
void show_chat_screen() {
    gtk_widget_hide(choice_box);
    gtk_widget_hide(login_box);
    gtk_widget_hide(register_box);
    gtk_widget_show(chat_box);
}

// Main programme
int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "MyDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // Create vertical container for the entire window
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // ===== Selection screen =====
    choice_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), choice_box, TRUE, TRUE, 0);
    
    GtkWidget *welcome_label = gtk_label_new("Bienvenue sur MyDiscord");
    gtk_widget_set_margin_top(welcome_label, 50);
    gtk_widget_set_margin_bottom(welcome_label, 30);
    gtk_box_pack_start(GTK_BOX(choice_box), welcome_label, FALSE, FALSE, 0);
    
    GtkWidget *choice_login_button = gtk_button_new_with_label("Connexion");
    gtk_widget_set_margin_bottom(choice_login_button, 10);
    gtk_box_pack_start(GTK_BOX(choice_box), choice_login_button, FALSE, FALSE, 0);
    g_signal_connect(choice_login_button, "clicked", G_CALLBACK(on_choice_login_clicked), NULL);
    
    GtkWidget *choice_register_button = gtk_button_new_with_label("Inscription");
    gtk_box_pack_start(GTK_BOX(choice_box), choice_register_button, FALSE, FALSE, 0);
    g_signal_connect(choice_register_button, "clicked", G_CALLBACK(on_choice_register_clicked), NULL);
    
    // ===== Connection screen =====
    login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), login_box, TRUE, TRUE, 0);
    
    GtkWidget *login_title = gtk_label_new("Connection");
    gtk_widget_set_margin_top(login_title, 30);
    gtk_widget_set_margin_bottom(login_title, 20);
    gtk_box_pack_start(GTK_BOX(login_box), login_title, FALSE, FALSE, 0);
    
    GtkWidget *email_label = gtk_label_new("Email:");
    gtk_box_pack_start(GTK_BOX(login_box), email_label, FALSE, FALSE, 0);
    
    login_email = gtk_entry_new();
    gtk_widget_set_margin_bottom(login_email, 10);
    gtk_box_pack_start(GTK_BOX(login_box), login_email, FALSE, FALSE, 0);
    
    GtkWidget *password_label = gtk_label_new("Password:");
    gtk_box_pack_start(GTK_BOX(login_box), password_label, FALSE, FALSE, 0);
    
    login_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(login_password), FALSE); // Hide password
    gtk_widget_set_margin_bottom(login_password, 20);
    gtk_box_pack_start(GTK_BOX(login_box), login_password, FALSE, FALSE, 0);
    
    GtkWidget *login_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(login_box), login_button_box, FALSE, FALSE, 0);
    
    GtkWidget *login_submit_button = gtk_button_new_with_label("Connect");
    gtk_box_pack_start(GTK_BOX(login_button_box), login_submit_button, TRUE, FALSE, 0);
    g_signal_connect(login_submit_button, "clicked", G_CALLBACK(on_login_submit_clicked), NULL);
    
    GtkWidget *login_back_button = gtk_button_new_with_label("Back");
    gtk_box_pack_start(GTK_BOX(login_button_box), login_back_button, FALSE, FALSE, 0);
    g_signal_connect(login_back_button, "clicked", G_CALLBACK(on_login_back_clicked), NULL);
    
    // ===== Registration screen =====
    register_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), register_box, TRUE, TRUE, 0);
    
    GtkWidget *register_title = gtk_label_new("Registration");
    gtk_widget_set_margin_top(register_title, 30);
    gtk_widget_set_margin_bottom(register_title, 20);
    gtk_box_pack_start(GTK_BOX(register_box), register_title, FALSE, FALSE, 0);
    
    GtkWidget *firstname_label = gtk_label_new("First name:");
    gtk_box_pack_start(GTK_BOX(register_box), firstname_label, FALSE, FALSE, 0);
    
    register_firstname = gtk_entry_new();
    gtk_widget_set_margin_bottom(register_firstname, 10);
    gtk_box_pack_start(GTK_BOX(register_box), register_firstname, FALSE, FALSE, 0);
    
    GtkWidget *lastname_label = gtk_label_new("Name:");
    gtk_box_pack_start(GTK_BOX(register_box), lastname_label, FALSE, FALSE, 0);
    
    register_lastname = gtk_entry_new();
    gtk_widget_set_margin_bottom(register_lastname, 10);
    gtk_box_pack_start(GTK_BOX(register_box), register_lastname, FALSE, FALSE, 0);
    
    GtkWidget *register_email_label = gtk_label_new("Email:");
    gtk_box_pack_start(GTK_BOX(register_box), register_email_label, FALSE, FALSE, 0);
    
    register_email = gtk_entry_new();
    gtk_widget_set_margin_bottom(register_email, 10);
    gtk_box_pack_start(GTK_BOX(register_box), register_email, FALSE, FALSE, 0);
    
    GtkWidget *register_password_label = gtk_label_new("Password:");
    gtk_box_pack_start(GTK_BOX(register_box), register_password_label, FALSE, FALSE, 0);
    
    register_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(register_password), FALSE); // Hide password
    gtk_widget_set_margin_bottom(register_password, 20);
    gtk_box_pack_start(GTK_BOX(register_box), register_password, FALSE, FALSE, 0);
    
    GtkWidget *register_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(register_box), register_button_box, FALSE, FALSE, 0);
    
    GtkWidget *register_submit_button = gtk_button_new_with_label("Register");
    gtk_box_pack_start(GTK_BOX(register_button_box), register_submit_button, TRUE, FALSE, 0);
    g_signal_connect(register_submit_button, "clicked", G_CALLBACK(on_register_submit_clicked), NULL);
    
    GtkWidget *register_back_button = gtk_button_new_with_label("Back");
    gtk_box_pack_start(GTK_BOX(register_button_box), register_back_button, FALSE, FALSE, 0);
    g_signal_connect(register_back_button, "clicked", G_CALLBACK(on_register_back_clicked), NULL);
    
    // ===== Chat screen =====
    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), chat_box, TRUE, TRUE, 0);
    
    // Messages display area
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(chat_box), scroll, TRUE, TRUE, 0);
    
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    
    // Input area and send button
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(chat_box), input_box, FALSE, FALSE, 0);
    
    entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_box_pack_start(GTK_BOX(input_box), entry, TRUE, TRUE, 0);
    g_signal_connect(entry, "key-press-event", G_CALLBACK(on_key_press), NULL);
    
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(input_box), send_button, FALSE, FALSE, 0);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_clicked), NULL);
    
    // Hide other screens
    gtk_widget_hide(login_box);
    gtk_widget_hide(register_box);
    gtk_widget_hide(chat_box);
    
    // Connection to server
    if (!init_connection()) {
        show_error_dialog("Impossible to connect to server.");
        return 1;
    }
    
    // Start reception thread
    start_receive_thread();
    
    // Display window
    gtk_widget_show_all(window);
    
    // Display only selection screen when starting
    show_choice_screen();
    
    // Start main loop
    gtk_main();
    
    return 0;
}