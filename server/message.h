#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>
#include <time.h>

// Structure pour représenter un message
typedef struct {
    int id;             // ID dans la base de données
    int user_id;        // ID de l'expéditeur
    int channel_id;     // ID du canal
    char content[1024]; // Contenu du message
    time_t timestamp;   // Horodatage
    bool is_encrypted;  // Si le message est chiffré
} message_t;

// Fonctions pour gérer les messages
bool save_message(int user_id, int channel_id, const char *content);
bool get_message_by_id(int message_id, message_t *message);
bool delete_message(int message_id, int user_id);
bool add_reaction(int message_id, int user_id, const char *emoji);
bool remove_reaction(int message_id, int user_id, const char *emoji);

#endif