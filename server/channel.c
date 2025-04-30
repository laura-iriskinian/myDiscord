#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "channel.h"
#include "database.h"
#include "user.h"
#include "server.h"

// Ajouter un nouveau canal
int add_channel(const char *name, bool is_private, int creator_id) {
    char query[200];
    sprintf(query, 
            "INSERT INTO channels (name, is_private, creator_id) "
            "VALUES ('%s', %s, %d) RETURNING id",
            name, is_private ? "TRUE" : "FALSE", creator_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return 0;
    }
    
    int channel_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    // Si c'est un canal privé, ajouter automatiquement le créateur
    if (is_private && channel_id > 0) {
        add_user_to_channel(creator_id, channel_id);
    }
    
    return channel_id;
}

// Supprimer un canal
bool remove_channel(int channel_id) {
    // D'abord, supprimer toutes les entrées de la table channel_users
    char query[200];
    sprintf(query, "DELETE FROM channel_users WHERE channel_id=%d", channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    PQclear(result);
    
    // Maintenant, supprimer le canal lui-même
    sprintf(query, "DELETE FROM channels WHERE id=%d", channel_id);
    
    result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Vérifier si un canal existe
bool channel_exists(int channel_id) {
    char query[100];
    sprintf(query, "SELECT id FROM channels WHERE id=%d", channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    bool exists = (PQntuples(result) > 0);
    PQclear(result);
    
    return exists;
}

// Récupérer le nom d'un canal
bool get_channel_name(int channel_id, char *name) {
    char query[100];
    sprintf(query, "SELECT name FROM channels WHERE id=%d", channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    strcpy(name, PQgetvalue(result, 0, 0));
    PQclear(result);
    
    return true;
}

// Récupérer les informations d'un canal
bool get_channel_by_id(int channel_id, channel_t *channel) {
    char query[100];
    sprintf(query, "SELECT * FROM channels WHERE id=%d", channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Remplir la structure canal
    channel->id = atoi(PQgetvalue(result, 0, 0));
    strcpy(channel->name, PQgetvalue(result, 0, 1));
    channel->is_private = strcmp(PQgetvalue(result, 0, 2), "t") == 0;
    channel->creator_id = atoi(PQgetvalue(result, 0, 3));
    
    PQclear(result);
    return true;
}

// Vérifier si un utilisateur peut accéder à un canal
bool can_access_channel(int user_id, int channel_id) {
    channel_t channel;
    if (!get_channel_by_id(channel_id, &channel)) {
        return false;
    }
    
    // Si c'est un canal public, tout le monde peut y accéder
    if (!channel.is_private) {
        return true;
    }
    
    // Si c'est le créateur, il a accès
    if (channel.creator_id == user_id) {
        return true;
    }
    
    // Sinon, vérifier dans la table channel_users
    char query[200];
    sprintf(query, "SELECT user_id FROM channel_users WHERE channel_id=%d AND user_id=%d",
            channel_id, user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    bool has_access = (PQntuples(result) > 0);
    PQclear(result);
    
    return has_access;
}

// Vérifier si un utilisateur est administrateur d'un canal
bool is_channel_admin(int user_id, int channel_id) {
    // Si l'utilisateur est admin global, il est admin partout
    if (is_admin(user_id)) {
        return true;
    }
    
    // Vérifier si c'est le créateur du canal
    channel_t channel;
    if (get_channel_by_id(channel_id, &channel) && channel.creator_id == user_id) {
        return true;
    }
    
    return false;
}

// Ajouter un utilisateur à un canal privé
bool add_user_to_channel(int user_id, int channel_id) {
    channel_t channel;
    
    // Vérifier que le canal existe et est privé
    if (!get_channel_by_id(channel_id, &channel) || !channel.is_private) {
        return false;
    }
    
    // Vérifier si l'utilisateur est déjà dans le canal
    char query[200];
    sprintf(query, "SELECT user_id FROM channel_users WHERE channel_id=%d AND user_id=%d",
            channel_id, user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    // Si l'utilisateur est déjà dans le canal, pas besoin de l'ajouter
    if (PQntuples(result) > 0) {
        PQclear(result);
        return true;
    }
    
    PQclear(result);
    
    // Ajouter l'utilisateur au canal
    sprintf(query, "INSERT INTO channel_users (channel_id, user_id) VALUES (%d, %d)",
            channel_id, user_id);
    
    result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Retirer un utilisateur d'un canal privé
bool remove_user_from_channel(int user_id, int channel_id) {
    channel_t channel;
    
    // Vérifier que le canal existe et est privé
    if (!get_channel_by_id(channel_id, &channel) || !channel.is_private) {
        return false;
    }
    
    // Empêcher la suppression du créateur
    if (channel.creator_id == user_id) {
        return false;
    }
    
    // Supprimer l'utilisateur du canal
    char query[200];
    sprintf(query, "DELETE FROM channel_users WHERE channel_id=%d AND user_id=%d",
            channel_id, user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Envoyer l'historique des messages d'un canal à un client
void send_channel_history(int client_id, int channel_id) {
    // Récupérer les 50 derniers messages
    char query[300];
    sprintf(query, 
            "SELECT m.content, u.username, m.timestamp "
            "FROM messages m "
            "JOIN users u ON m.user_id = u.id "
            "WHERE m.channel_id = %d "
            "ORDER BY m.timestamp DESC "
            "LIMIT 50",
            channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return;
    }
    
    // Envoyer un en-tête
    char channel_name[50];
    get_channel_name(channel_id, channel_name);
    
    char header[100];
    sprintf(header, "--- Historique du canal '%s' ---", channel_name);
    send_to_client(client_id, header);
    
    // Parcourir les résultats du plus ancien au plus récent
    int count = PQntuples(result);
    for (int i = count - 1; i >= 0; i--) {
        char message[BUFFER_SIZE];
        sprintf(message, "[%s] %s: %s", 
                PQgetvalue(result, i, 2),  // timestamp
                PQgetvalue(result, i, 1),  // username
                PQgetvalue(result, i, 0)); // content
        
        send_to_client(client_id, message);
    }
    
    send_to_client(client_id, "--- Fin de l'historique ---");
    
    PQclear(result);
}