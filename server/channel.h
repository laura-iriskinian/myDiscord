#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>

// Structure pour représenter un canal
typedef struct {
    int id;                 // ID dans la base de données
    char name[50];          // Nom du canal
    bool is_private;        // Canal privé ou public
    int creator_id;         // ID de l'utilisateur qui a créé le canal
} channel_t;

// Fonctions pour gérer les canaux
int add_channel(const char *name, bool is_private, int creator_id);
bool remove_channel(int channel_id);
bool channel_exists(int channel_id);
bool get_channel_name(int channel_id, char *name);
bool get_channel_by_id(int channel_id, channel_t *channel);

// Gestion des permissions
bool can_access_channel(int user_id, int channel_id);
bool is_channel_admin(int user_id, int channel_id);
bool add_user_to_channel(int user_id, int channel_id);
bool remove_user_from_channel(int user_id, int channel_id);

// Récupération des messages
void send_channel_history(int client_id, int channel_id);

#endif 