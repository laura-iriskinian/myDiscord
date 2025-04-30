#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "database.h"
#include "channel.h"
#include "user.h"
#include "crypto.h"

// Sauvegarder un message dans la base de données
bool save_message(int user_id, int channel_id, const char *content) {
    // Vérifier si l'utilisateur peut poster dans ce canal
    if (!can_access_channel(user_id, channel_id)) {
        return false;
    }
    
    // Vérifier si c'est un message privé (commence par /p)
    bool is_private = (strncmp(content, "/p ", 3) == 0);
    const char *msg_content = is_private ? content + 3 : content;
    
    // Chiffrer le contenu si c'est un message privé
    char *stored_content;
    if (is_private) {
        stored_content = encrypt_message(msg_content);
        if (!stored_content) return false;
    } else {
        stored_content = strdup(msg_content);
    }
    
    // Insérer dans la base de données
    char query[1500];
    sprintf(query, 
            "INSERT INTO messages (user_id, channel_id, content, timestamp, is_encrypted) "
            "VALUES (%d, %d, '%s', NOW(), %s)",
            user_id, channel_id, stored_content, is_private ? "TRUE" : "FALSE");
    
    free(stored_content);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Récupérer un message par son ID
bool get_message_by_id(int message_id, message_t *message) {
    char query[200];
    sprintf(query, "SELECT * FROM messages WHERE id=%d", message_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Remplir la structure message
    message->id = atoi(PQgetvalue(result, 0, 0));
    message->user_id = atoi(PQgetvalue(result, 0, 1));
    message->channel_id = atoi(PQgetvalue(result, 0, 2));
    strcpy(message->content, PQgetvalue(result, 0, 3));
    
    // Convertir le timestamp PostgreSQL en time_t
    struct tm tm_time;
    strptime(PQgetvalue(result, 0, 4), "%Y-%m-%d %H:%M:%S", &tm_time);
    message->timestamp = mktime(&tm_time);
    
    message->is_encrypted = strcmp(PQgetvalue(result, 0, 5), "t") == 0;
    
    // Si le message est chiffré, le déchiffrer
    if (message->is_encrypted) {
        char *decrypted = decrypt_message(message->content);
        if (decrypted) {
            strcpy(message->content, decrypted);
            free(decrypted);
        }
    }
    
    PQclear(result);
    return true;
}

// Supprimer un message
bool delete_message(int message_id, int user_id) {
    // Vérifier que l'utilisateur est l'auteur du message ou un modérateur
    message_t message;
    if (!get_message_by_id(message_id, &message)) {
        return false;
    }
    
    if (message.user_id != user_id && !is_moderator(user_id) && 
        !is_channel_admin(user_id, message.channel_id)) {
        return false;
    }
    
    // Supprimer d'abord les réactions
    char query[200];
    sprintf(query, "DELETE FROM reactions WHERE message_id=%d", message_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    
    // Maintenant supprimer le message
    sprintf(query, "DELETE FROM messages WHERE id=%d", message_id);
    
    result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Ajouter une réaction à un message
bool add_reaction(int message_id, int user_id, const char *emoji) {
    // Vérifier que le message existe
    message_t message;
    if (!get_message_by_id(message_id, &message)) {
        return false;
    }
    
    // Vérifier que l'utilisateur a accès au canal
    if (!can_access_channel(user_id, message.channel_id)) {
        return false;
    }
    
    // Vérifier si la réaction existe déjà
    char query[300];
    sprintf(query, 
            "SELECT id FROM reactions WHERE message_id=%d AND user_id=%d AND emoji='%s'",
            message_id, user_id, emoji);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    // Si la réaction existe déjà, on ne fait rien
    if (PQntuples(result) > 0) {
        PQclear(result);
        return true;
    }
    
    PQclear(result);
    
    // Ajouter la réaction
    sprintf(query, 
            "INSERT INTO reactions (message_id, user_id, emoji, timestamp) "
            "VALUES (%d, %d, '%s', NOW())",
            message_id, user_id, emoji);
    
    result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Supprimer une réaction
bool remove_reaction(int message_id, int user_id, const char *emoji) {
    char query[300];
    sprintf(query, 
            "DELETE FROM reactions WHERE message_id=%d AND user_id=%d AND emoji='%s'",
            message_id, user_id, emoji);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}