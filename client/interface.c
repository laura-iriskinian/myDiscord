#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include "interface.h"
#include "client.h"

// Définir nos propres structures pour l'interface utilisateur
// pour éviter les dépendances aux fichiers du serveur

// Structure pour représenter un canal dans l'interface
typedef struct {
    int id;             // ID du canal
    char name[50];      // Nom du canal
    bool is_private;    // Est-ce un canal privé
} UIChannel;

// Structure pour représenter un message dans l'interface
typedef struct {
    int id;             // ID du message
    char sender[50];    // Nom de l'expéditeur
    char content[1024]; // Contenu du message
    char timestamp[20]; // Horodatage formaté
} UIMessage;

// Structures pour les données de l'application
typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *chat_text_view;
    GtkWidget *chat_history;
    GtkWidget *main_chat_area;
    int current_channel_id;
} AppData;

// Variable globale pour les données de l'application
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

// Construction des écrans
static GtkWidget* build_choice_screen(void);
static GtkWidget* build_signin_screen(void);
static GtkWidget* build_register_screen(void);
static GtkWidget* build_chat_screen(void);

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
        
        // Initialiser le canal courant (canal principal par défaut)
        app_data.current_channel_id = 1;
        
        // Mettre à jour les messages
        update_channel_messages(app_data.current_channel_id);
    }
}

// Fonctions pour construire les différents écrans dans la fenêtre
static GtkWidget* build_choice_screen(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("Bienvenue sur MyDiscord");
    gtk_widget_add_css_class(label, "welcome-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *signin_button = gtk_button_new_with_label("Connexion");
    GtkWidget *register_button = gtk_button_new_with_label("Inscription");
    
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

    GtkWidget *label = gtk_label_new("Connexion");
    gtk_widget_add_css_class(label, "form-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *signin_button = gtk_button_new_with_label("Connexion");
    gtk_widget_add_css_class(signin_button, "my-button");
    
    // Stock the fields references
    g_object_set_data(G_OBJECT(signin_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(signin_button), "entry_password", entry_password);
    
    // Connect click manager
    g_signal_connect(signin_button, "clicked", G_CALLBACK(on_signin_button_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), signin_button);

    GtkWidget *back_button = gtk_button_new_with_label("Retour");
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

    GtkWidget *label = gtk_label_new("Inscription");
    gtk_widget_add_css_class(label, "form-title");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Nom");
    gtk_widget_add_css_class(entry_name, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_name);
    
    GtkWidget *entry_first_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_first_name), "Prénom");
    gtk_widget_add_css_class(entry_first_name, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_first_name);
    
    GtkWidget *entry_mail = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mail), "Email");
    gtk_widget_add_css_class(entry_mail, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_mail);
    
    GtkWidget *entry_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_password), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_widget_add_css_class(entry_password, "my-entry");
    gtk_box_append(GTK_BOX(box), entry_password);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *register_button = gtk_button_new_with_label("S'inscrire");
    gtk_widget_add_css_class(register_button, "my-button");

    // stock the references in the fields
    g_object_set_data(G_OBJECT(register_button), "entry_name", entry_name);
    g_object_set_data(G_OBJECT(register_button), "entry_first_name", entry_first_name);
    g_object_set_data(G_OBJECT(register_button), "entry_mail", entry_mail);
    g_object_set_data(G_OBJECT(register_button), "entry_password", entry_password);

    // Connect click manager
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), register_button);

    GtkWidget *back_button = gtk_button_new_with_label("Retour");
    gtk_widget_add_css_class(back_button, "my-button");
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(button_box), back_button);
    
    gtk_box_append(GTK_BOX(box), button_box);

    return box;
}

static GtkWidget* build_chat_screen(void) {
    // Création du layout principal horizontal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // 1. Panneau des serveurs (à gauche)
    GtkWidget *servers_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(servers_panel, 70, -1);
    gtk_widget_add_css_class(servers_panel, "servers");
    
    // Pour l'instant, nous n'avons qu'un seul serveur
    GtkWidget *server_btn = gtk_button_new_with_label("MD");
    gtk_widget_add_css_class(server_btn, "server-button");
    gtk_box_append(GTK_BOX(servers_panel), server_btn);
    
    // 2. Panneau des canaux (à côté du panneau des serveurs)
    GtkWidget *channels_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(channels_panel, 200, -1);
    gtk_widget_add_css_class(channels_panel, "conversations");
    
    // Récupérer les canaux depuis le backend (à implémenter)
    // Pour l'instant, ajouter quelques canaux de test
    const char *channel_names[] = {"Général", "Annonces", "Entraide", "Général-privé", "Discussions"};
    int channel_ids[] = {1, 2, 3, 4, 5};
    
    for (int i = 0; i < 5; i++) {
        GtkWidget *channel_btn = gtk_button_new_with_label(channel_names[i]);
        gtk_widget_add_css_class(channel_btn, "conversation-button");
        
        // Allouer et stocker l'ID du canal pour pouvoir l'utiliser dans le callback
        int *id_ptr = g_new(int, 1);
        *id_ptr = channel_ids[i];
        g_signal_connect(channel_btn, "clicked", G_CALLBACK(on_channel_clicked), id_ptr);
        
        gtk_box_append(GTK_BOX(channels_panel), channel_btn);
    }
    
    // 3. Panneau de chat (au centre)
    GtkWidget *chat_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(chat_panel, "chat");
    gtk_widget_set_hexpand(chat_panel, TRUE);
    gtk_widget_set_vexpand(chat_panel, TRUE);
    
    // Zone de messages (scrollable)
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll_window, TRUE);
    
    app_data.main_chat_area = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_data.main_chat_area), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app_data.main_chat_area), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_data.main_chat_area), GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(app_data.main_chat_area, "chat-area");
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), app_data.main_chat_area);
    gtk_box_append(GTK_BOX(chat_panel), scroll_window);
    
    // Zone de saisie de message
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *msg_scroll = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(msg_scroll, TRUE);
    
    app_data.chat_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_data.chat_text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(app_data.chat_text_view, "chat-entry");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(msg_scroll), app_data.chat_text_view);
    
    GtkWidget *send_button = gtk_button_new_with_label("Envoyer");
    gtk_widget_add_css_class(send_button, "my-button");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message_clicked), NULL);
    
    gtk_box_append(GTK_BOX(input_box), msg_scroll);
    gtk_box_append(GTK_BOX(input_box), send_button);
    
    gtk_box_append(GTK_BOX(chat_panel), input_box);
    
    // 4. Panneau de profil (à droite)
    GtkWidget *profile_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(profile_panel, 140, -1);
    gtk_widget_add_css_class(profile_panel, "profile");
    
    GtkWidget *profile_label = gtk_label_new("Profil");
    gtk_widget_add_css_class(profile_label, "profile-label");
    gtk_box_append(GTK_BOX(profile_panel), profile_label);
    
    GtkWidget *theme_label = gtk_label_new("Choisir un thème :");
    gtk_widget_add_css_class(theme_label, "theme-label");
    gtk_box_append(GTK_BOX(profile_panel), theme_label);
    
    // Boutons de thèmes
    const char *themes[] = {"theme-dark", "theme-light", "theme-violet"};
    const char *labels[] = {"Thème Sombre", "Thème Clair", "Thème Violet"};
    
    for (int i = 0; i < 3; i++) {
        GtkWidget *theme_btn = gtk_button_new_with_label(labels[i]);
        gtk_widget_add_css_class(theme_btn, "theme-button");
        gtk_widget_set_name(theme_btn, themes[i]);
        g_signal_connect(theme_btn, "clicked", G_CALLBACK(on_theme_button_clicked), NULL);
        gtk_box_append(GTK_BOX(profile_panel), theme_btn);
    }
    
    // Bouton de déconnexion
    GtkWidget *logout_button = gtk_button_new_with_label("Déconnexion");
    gtk_widget_add_css_class(logout_button, "my-button");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(show_choice_screen), NULL);
    gtk_box_append(GTK_BOX(profile_panel), logout_button);
    
    // Assembler tous les panneaux
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
                              "Une erreur s'est produite lors de l'authentification.\nVeuillez réessayer avec des identifiants valides.";
    
    // Créer une boîte de dialogue moderne (non dépréciée)
    if (app_data.window != NULL) {
        GtkWidget *dialog = gtk_alert_dialog_new("%s", safe_message);
        gtk_alert_dialog_show(GTK_ALERT_DIALOG(dialog), app_data.window);
        g_object_unref(dialog);
    } else {
        g_print("Error: %s\n", safe_message);
    }
}

// Gestion du changement de thème
static void on_theme_button_clicked(GtkButton *button, gpointer user_data) {
    const char *theme = gtk_widget_get_name(GTK_WIDGET(button));

    if (app_data.window != NULL) {
        // Retirer tous les anciens thèmes
        gtk_widget_remove_css_class(app_data.window, "theme-dark");
        gtk_widget_remove_css_class(app_data.window, "theme-light");
        gtk_widget_remove_css_class(app_data.window, "theme-violet");

        // Appliquer le nouveau thème
        gtk_widget_add_css_class(app_data.window, theme);
    }
}

// Gestion du changement de canal
static void on_channel_clicked(GtkButton *button, gpointer id_ptr) {
    if (id_ptr != NULL) {
        int channel_id = *(int*)id_ptr;
        
        // Mettre à jour le canal courant
        app_data.current_channel_id = channel_id;
        
        // Commander au serveur de rejoindre le canal
        char command[BUFFER_SIZE];
        sprintf(command, "/join %d", channel_id);
        send_message_to_server(command);
        
        // Mettre à jour les messages affichés
        update_channel_messages(channel_id);
    }
}

// Mise à jour des messages du canal
static void update_channel_messages(int channel_id) {
    if (app_data.main_chat_area != NULL) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.main_chat_area));
        
        // Effacer le contenu actuel
        gtk_text_buffer_set_text(buffer, "", -1);
        
        // En attente des messages du serveur, un message de chargement est affiché
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, "Chargement des messages...\n", -1);
        
        // Les messages réels seront chargés par la fonction de réception des messages du serveur
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
        show_error_dialog("Erreur interne de l'application.");
        return;
    }
    
    const char *name = gtk_editable_get_text(GTK_EDITABLE(entry_name));
    const char *first_name = gtk_editable_get_text(GTK_EDITABLE(entry_first_name));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(name) == 0 || strlen(first_name) == 0 || strlen(email) == 0 || strlen(password) == 0) {
        show_error_dialog("Veuillez remplir tous les champs");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Adresse email invalide");
        return;
    }
    
    // Validate password
    if (!validate_password(password)) {
        show_error_dialog("Le mot de passe doit contenir au moins 10 caractères, "
                         "une lettre majuscule, une lettre minuscule, "
                         "un chiffre et un caractère spécial");
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
        show_error_dialog("Erreur interne de l'application.");
        return;
    }
    
    const char *email = gtk_editable_get_text(GTK_EDITABLE(entry_mail));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(entry_password));
    
    // Check that all fields are complete
    if (strlen(email) == 0 || strlen(password) == 0) {
        show_error_dialog("Veuillez remplir tous les champs");
        return;
    }
    
    // Validate email
    if (!validate_email(email)) {
        show_error_dialog("Adresse email invalide");
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

// Fonction pour ajouter un message au chat
void add_message_to_chat(const char *message) {
    if (app_data.main_chat_area == NULL) return;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data.main_chat_area));
    GtkTextIter iter;
    
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, message, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    
    // Faire défiler vers le bas pour toujours voir le dernier message
    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app_data.main_chat_area), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(buffer, mark);
}

// Main window 
void build_main_window(GtkApplication *app) {
    // Initialiser la structure AppData
    memset(&app_data, 0, sizeof(AppData));
    
    // Créer la fenêtre principale
    app_data.window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data.window), "MyDiscord Client");
    gtk_window_set_default_size(GTK_WINDOW(app_data.window), 1200, 800);
    
    // Appliquer le thème par défaut
    gtk_widget_add_css_class(app_data.window, "theme-dark");

    // Créer le stack pour les différentes vues
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

// Fonction pour traiter les messages entrants du serveur
void process_incoming_message(const char *message) {
    // Si nous sommes dans l'écran de chat, afficher le message
    if (app_data.stack != NULL && 
        gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)) &&
        strcmp(gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)), "chat_screen") == 0) {
        
        // Ajouter le message à la zone de chat
        add_message_to_chat(message);
    }
}

// Fonction pour créer un nouveau canal
void create_new_channel(const char *name, bool is_private) {
    char command[BUFFER_SIZE];
    if (is_private) {
        sprintf(command, "/create %s private", name);
    } else {
        sprintf(command, "/create %s", name);
    }
    send_message_to_server(command);
}

// Fonction pour supprimer un canal
void delete_channel(int channel_id) {
    char command[BUFFER_SIZE];
    sprintf(command, "/delete %d", channel_id);
    send_message_to_server(command);
}

// Fonction pour mettre à jour l'interface utilisateur
// Cette fonction peut être appelée périodiquement ou à des moments clés
void update_ui() {
    // Si nous sommes dans l'écran de chat, mettre à jour les canaux et les messages
    if (app_data.stack != NULL && 
        gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)) &&
        strcmp(gtk_stack_get_visible_child_name(GTK_STACK(app_data.stack)), "chat_screen") == 0) {
        
        // Mettre à jour les messages du canal courant
        update_channel_messages(app_data.current_channel_id);
    }
}