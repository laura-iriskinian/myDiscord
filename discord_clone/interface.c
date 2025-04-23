// // #include <gtk/gtk.h>

// // static void on_send_message(GtkEntry *entry, gpointer user_data) {
// //     const char *message = gtk_entry_get_text(entry);
// //     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(user_data));
// //     gtk_text_buffer_insert_at_cursor(buffer, message, -1);
// //     gtk_entry_set_text(entry, "");
// // }

// // static void on_add_server(GtkButton *button, gpointer user_data) {
// //     g_print("Ajouter un serveur (fonctionnalité à implémenter)\n");
// // }

// // int main(int argc, char *argv[]) {
// //     GtkApplication *app;
// //     int status;

// //     app = gtk_application_new("com.example.mini_discord", G_APPLICATION_FLAGS_NONE);
    
// //     g_signal_connect(app, "activate", G_CALLBACK(gtk_window_present), window);

// //     status = g_application_run(G_APPLICATION(app), argc, argv);
// //     g_object_unref(app);

// //     return status;
// // }

// // static void activate(GtkApplication *app, gpointer user_data) {
// //     GtkWidget *window;
// //     GtkWidget *splitter;
// //     GtkWidget *left_panel;
// //     GtkWidget *right_panel;
// //     GtkWidget *server_list;
// //     GtkWidget *add_server_button;
// //     GtkWidget *message_view;
// //     GtkWidget *message_scroll;
// //     GtkWidget *message_entry;

// //     // Créer la fenêtre principale
// //     window = gtk_application_window_new(app);
// //     gtk_window_set_title(GTK_WINDOW(window), "Mini Discord");
// //     gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

// //     // Créer un splitter (division en 2)
// //     splitter = gtk_splitter_new();
// //     gtk_window_set_child(GTK_WINDOW(window), splitter);

// //     // Partie gauche : Liste des serveurs et canaux
// //     left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
// //     gtk_splitter_set_start_child(GTK_SPLITTER(splitter), left_panel);

// //     server_list = gtk_list_box_new();
// //     gtk_list_box_set_selection_mode(GTK_LIST_BOX(server_list), GTK_SELECTION_SINGLE);
// //     gtk_box_append(GTK_BOX(left_panel), server_list);

// //     add_server_button = gtk_button_new_with_label("Ajouter Serveur");
// //     g_signal_connect(add_server_button, "clicked", G_CALLBACK(on_add_server), NULL);
// //     gtk_box_append(GTK_BOX(left_panel), add_server_button);

// //     // Partie droite : Liste des messages et chat
// //     right_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
// //     gtk_splitter_set_end_child(GTK_SPLITTER(splitter), right_panel);

// //     message_view = gtk_text_view_new();
// //     gtk_text_view_set_editable(GTK_TEXT_VIEW(message_view), FALSE);
// //     message_scroll = gtk_scrolled_window_new();
// //     gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(message_scroll), message_view);
// //     gtk_box_append(GTK_BOX(right_panel), message_scroll);

// //     message_entry = gtk_entry_new();
// //     gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Écrire un message...");
// //     g_signal_connect(message_entry, "activate", G_CALLBACK(on_send_message), message_view);
// //     gtk_box_append(GTK_BOX(right_panel), message_entry);

// //     gtk_widget_show_all(window);
// // }


// #include <gtk/gtk.h>

// static void activate(GtkApplication *app, gpointer user_data) {
//     GtkWidget *window;
//     GtkWidget *entry;
//     GtkWidget *button;
//     GtkWidget *box;

//     window = gtk_application_window_new(app);
//     gtk_window_set_title(GTK_WINDOW(window), "Mini Discord");
//     gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

//     box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
//     gtk_window_set_child(GTK_WINDOW(window), box);

//     entry = gtk_entry_new();
//     gtk_box_append(GTK_BOX(box), entry);

//     button = gtk_button_new_with_label("Envoyer");
//     gtk_box_append(GTK_BOX(box), button);

//     gtk_window_present(GTK_WINDOW(window));
// }

// int main(int argc, char **argv) {
//     GtkApplication *app;
//     int status;

//     app = gtk_application_new("com.example.minidiscord", G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//     status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);

//     return status;
// }


// #include <gtk/gtk.h>

// static void activate(GtkApplication *app, gpointer user_data) {
//     GtkWidget *window;
//     GtkWidget *main_box;
//     GtkWidget *server_list;
//     GtkWidget *channel_list;
//     GtkWidget *chat_area;
//     GtkWidget *chat_box;
//     GtkWidget *entry;
//     GtkWidget *send_button;

//     // Fenêtre principale
//     window = gtk_application_window_new(app);
//     gtk_window_set_title(GTK_WINDOW(window), "Mini Discord");
//     gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

//     // Box horizontale pour layout principal
//     main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
//     gtk_window_set_child(GTK_WINDOW(window), main_box);

//     // Zone serveurs
//     server_list = gtk_list_box_new();
//     gtk_widget_set_size_request(server_list, 80, -1);
//     gtk_box_append(GTK_BOX(main_box), server_list);
//     gtk_list_box_append(GTK_LIST_BOX(server_list), gtk_label_new("Serveur 1"));
//     gtk_list_box_append(GTK_LIST_BOX(server_list), gtk_label_new("Serveur 2"));

//     // Zone canaux
//     channel_list = gtk_list_box_new();
//     gtk_widget_set_size_request(channel_list, 150, -1);
//     gtk_box_append(GTK_BOX(main_box), channel_list);
//     gtk_list_box_append(GTK_LIST_BOX(channel_list), gtk_label_new("# général"));
//     gtk_list_box_append(GTK_LIST_BOX(channel_list), gtk_label_new("# code"));

//     // Zone de chat (verticale)
//     chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
//     gtk_box_append(GTK_BOX(main_box), chat_box);

//     // Affichage des messages (placeholder)
//     chat_area = gtk_text_view_new();
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_area), FALSE);
//     gtk_widget_set_vexpand(chat_area, TRUE);
//     gtk_box_append(GTK_BOX(chat_box), chat_area);

//     // Champ d'entrée + bouton envoyer
//     GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
//     entry = gtk_entry_new();
//     gtk_widget_set_hexpand(entry, TRUE);
//     send_button = gtk_button_new_with_label("Envoyer");
//     gtk_box_append(GTK_BOX(bottom_box), entry);
//     gtk_box_append(GTK_BOX(bottom_box), send_button);

//     gtk_box_append(GTK_BOX(chat_box), bottom_box);

//     gtk_window_present(GTK_WINDOW(window));
// }

// int main(int argc, char **argv) {
//     GtkApplication *app;
//     int status;

//     app = gtk_application_new("com.example.discordlike", G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//     load_css();
//     status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);
//     return status;
// }

// void load_css(void) {
//     GtkCssProvider *provider = gtk_css_provider_new();
//     gtk_css_provider_load_from_path(provider, "style.css");
//     gtk_style_context_add_provider_for_display(
//         gdk_display_get_default(),
//         GTK_STYLE_PROVIDER(provider),
//         GTK_STYLE_PROVIDER_PRIORITY_USER
//     );
// }

// ------------------- Version fonctionnelle couleurs mais pas organisation ----------

// #include <gtk/gtk.h>

// static void on_send_message(GtkButton *button, gpointer user_data) {
//     GtkEntry *entry = GTK_ENTRY(user_data);
//     const char *message = gtk_entry_get_text(entry);

//     if (g_strcmp0(message, "") != 0) {
//         g_print("Message envoyé : %s\n", message);
//         gtk_entry_set_text(entry, "");
//     }
// }

// static void activate(GtkApplication *app, gpointer user_data) {
//     GtkWidget *window;
//     GtkWidget *vbox;
//     GtkWidget *entry;
//     GtkWidget *send_button;
//     GtkWidget *text_view;

//     window = gtk_application_window_new(app);
//     gtk_window_set_title(GTK_WINDOW(window), "Mini Discord");
//     gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

//     // Layout vertical
//     vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_window_set_child(GTK_WINDOW(window), vbox);

//     // Zone de texte
//     text_view = gtk_text_view_new();
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
//     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
//     gtk_box_append(GTK_BOX(vbox), text_view);

//     // Entrée utilisateur
//     entry = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Écris ton message ici...");
//     gtk_box_append(GTK_BOX(vbox), entry);

//     // Bouton envoyer
//     send_button = gtk_button_new_with_label("Envoyer");
//     gtk_box_append(GTK_BOX(vbox), send_button);
//     g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message), entry);

//     // ===== CSS =====
//     GtkCssProvider *provider = gtk_css_provider_new();
//     gtk_css_provider_load_from_path(provider, "style.css", NULL);

//     GdkDisplay *display = gtk_widget_get_display(window);
//     gtk_style_context_add_provider_for_display(display,
//         GTK_STYLE_PROVIDER(provider),
//         GTK_STYLE_PROVIDER_PRIORITY_USER);

//     gtk_widget_show(window);
// }

// int main(int argc, char **argv) {
//     GtkApplication *app;
//     int status;

//     app = gtk_application_new("com.example.discord_clone", G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

//     status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);

//     return status;
// }

// #include <gtk/gtk.h>
// #include <gtk/gtkentry.h>  // Ajout de l'inclusion pour gtk_entry_get_text et gtk_entry_set_text

// // Fonction appelée lors de l'envoi du message
// void on_send_message(GtkWidget *button, gpointer user_data) {
//     GtkWidget *entry = GTK_WIDGET(user_data);

//     const char *message = gtk_editable_get_text(GTK_EDITABLE(entry));

//     if (strlen(message) > 0) {
//         g_print("Message envoyé : %s\n", message);
//         gtk_editable_set_text(GTK_EDITABLE(entry), "");  // Vide le champ
//     }
// }

// // Fonction d'activation de l'application
// static void activate(GtkApplication *app, gpointer user_data) {
//     // Créer la fenêtre principale
//     GtkWidget *window = gtk_application_window_new(app);
//     gtk_window_set_title(GTK_WINDOW(window), "Mini Discord");
//     gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

//     // Création du conteneur principal (box verticale)
//     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
//     gtk_window_set_child(GTK_WINDOW(window), box);

//     // Création d'un champ de texte (entry)
//     GtkWidget *entry = gtk_entry_new();
//     gtk_box_append(GTK_BOX(box), entry);

//     // Création d'un bouton pour envoyer le message
//     GtkWidget *send_button = gtk_button_new_with_label("Envoyer");
//     gtk_box_append(GTK_BOX(box), send_button);

//     // Connecte l'événement "clicked" du bouton à la fonction d'envoi de message
//     g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message), entry);

//     // Application du CSS à la fenêtre
//     GtkCssProvider *provider = gtk_css_provider_new();
//     gtk_css_provider_load_from_path(provider, "style.css");  // Correction ici
//     gtk_style_context_add_provider_for_display(
//         gdk_display_get_default(),
//         GTK_STYLE_PROVIDER(provider),
//         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
//     );

//     // Affiche la fenêtre
//     gtk_window_present(GTK_WINDOW(window));  // Utilisation de gtk_window_present à la place de gtk_widget_show
// }

// int main(int argc, char **argv) {
//     GtkApplication *app = gtk_application_new("com.example.mini_discord", G_APPLICATION_DEFAULT_FLAGS);  // Utilisation de G_APPLICATION_DEFAULT_FLAGS
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//     int status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);
//     return status;
// }
// ------------------- Version fonctionnelle couleurs mais pas organisation ----------

// #include <gtk/gtk.h>

// static void activate(GtkApplication *app, gpointer user_data) {
//     GtkWidget *window = gtk_application_window_new(app);
//     gtk_window_set_title(GTK_WINDOW(window), "Discord Clone");
//     gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

//     GtkCssProvider *provider = gtk_css_provider_new();
//     gtk_css_provider_load_from_path(provider, "style.css");
//     gtk_style_context_add_provider_for_display(gdk_display_get_default(),
//         GTK_STYLE_PROVIDER(provider),
//         GTK_STYLE_PROVIDER_PRIORITY_USER);

//     // Layout principal : horizontal
//     GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
//     gtk_window_set_child(GTK_WINDOW(window), main_box);

//     // Liste serveurs
//     GtkWidget *servers_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_set_size_request(servers_panel, 80, -1);
//     gtk_widget_add_css_class(servers_panel, "servers");

//     for (int i = 0; i < 5; i++) {
//         GtkWidget *server_btn = gtk_button_new_with_label("S");
//         gtk_widget_add_css_class(server_btn, "server-button");
//         gtk_box_append(GTK_BOX(servers_panel), server_btn);
//     }

//     // Liste conversations
//     GtkWidget *conversations_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_set_size_request(conversations_panel, 200, -1);
//     gtk_widget_add_css_class(conversations_panel, "conversations");

//     for (int i = 0; i < 8; i++) {
//         GtkWidget *conv_label = gtk_label_new("Général");
//         gtk_widget_add_css_class(conv_label, "conversation-label");
//         gtk_box_append(GTK_BOX(conversations_panel), conv_label);
//     }

//     // Chat
//     GtkWidget *chat_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_add_css_class(chat_panel, "chat");

//     GtkWidget *chat_area = gtk_text_view_new();
//     gtk_widget_set_vexpand(chat_area, TRUE);
//     gtk_widget_add_css_class(chat_area, "chat-area");

//     GtkWidget *entry = gtk_entry_new();
//     gtk_widget_add_css_class(entry, "chat-entry");

//     gtk_box_append(GTK_BOX(chat_panel), chat_area);
//     gtk_box_append(GTK_BOX(chat_panel), entry);

//     // Profil
//     GtkWidget *profile_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_widget_set_size_request(profile_panel, 180, -1);
//     gtk_widget_add_css_class(profile_panel, "profile");

//     GtkWidget *profile_label = gtk_label_new("Louis #1234");
//     gtk_widget_add_css_class(profile_label, "profile-label");
//     gtk_box_append(GTK_BOX(profile_panel), profile_label);

//     // Ajout des panneaux dans le layout principal
//     gtk_box_append(GTK_BOX(main_box), servers_panel);
//     gtk_box_append(GTK_BOX(main_box), conversations_panel);
//     gtk_box_append(GTK_BOX(main_box), chat_panel);
//     gtk_box_append(GTK_BOX(main_box), profile_panel);

//     gtk_window_present(GTK_WINDOW(window));
// }

// int main(int argc, char **argv) {
//     GtkApplication *app = gtk_application_new("com.discord.clone", G_APPLICATION_DEFAULT_FLAGS);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//     int status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);
//     return status;
// }


// Fonctionne aussi mais probleme organisation fenetres

#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Discord Clone");
    gtk_window_set_default_size(GTK_WINDOW(window), 1400, 800);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Colonne serveurs (étroite)
    GtkWidget *servers_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(servers_panel, 70, -1);
    gtk_widget_add_css_class(servers_panel, "servers");

    for (int i = 0; i < 5; i++) {
        GtkWidget *server_btn = gtk_button_new_with_label("S");
        gtk_widget_add_css_class(server_btn, "server-button");
        gtk_box_append(GTK_BOX(servers_panel), server_btn);
    }

    // Colonne conversations (moyenne)
    GtkWidget *conversations_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(conversations_panel, 200, -1);
    gtk_widget_add_css_class(conversations_panel, "conversations");

    for (int i = 0; i < 8; i++) {
        GtkWidget *conv_label = gtk_label_new("général");
        gtk_widget_add_css_class(conv_label, "conversation-label");
        gtk_box_append(GTK_BOX(conversations_panel), conv_label);
    }

    GtkWidget *chat_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(chat_panel, "chat");
    gtk_widget_set_hexpand(chat_panel, TRUE);
    gtk_widget_set_vexpand(chat_panel, TRUE);
    
    // Zone de texte avec messages
    GtkWidget *chat_area = gtk_text_view_new();
    gtk_widget_set_vexpand(chat_area, TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_area), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chat_area), FALSE);
    gtk_widget_add_css_class(chat_area, "chat-area");
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_area));
    gtk_text_buffer_set_text(buffer,
        "Louis: Salut, let's go créer un chat discord like !\n"
        "Nuage : Cest pas évident du  tout ! good luck \n"
        "Louis: Merci ! Il nous en faudra du courage ! \n",
        -1
    );
    
    // Champ de saisie
    GtkWidget *entry = gtk_entry_new();
    gtk_widget_add_css_class(entry, "chat-entry");
    
    // Ajout au panneau
    gtk_box_append(GTK_BOX(chat_panel), chat_area);
    gtk_box_append(GTK_BOX(chat_panel), entry);
    

    // Colonne profil (étroite)
    GtkWidget *profile_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(profile_panel, 140, -1);
    gtk_widget_add_css_class(profile_panel, "profile");

    GtkWidget *profile_label = gtk_label_new("Louis #1234");
    gtk_widget_add_css_class(profile_label, "profile-label");
    gtk_box_append(GTK_BOX(profile_panel), profile_label);

    // Ajout dans la box principale
    gtk_box_append(GTK_BOX(main_box), servers_panel);
    gtk_box_append(GTK_BOX(main_box), conversations_panel);
    gtk_box_append(GTK_BOX(main_box), chat_panel);
    gtk_box_append(GTK_BOX(main_box), profile_panel);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.discord.clone", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
    