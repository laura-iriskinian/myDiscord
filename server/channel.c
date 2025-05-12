#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "channel.h"
#include "database.h"
#include "user.h"
#include "server.h"

// Add a new channel
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
    
    // If private channel, automatically add the creator
    if (is_private && channel_id > 0) {
        add_user_to_channel(creator_id, channel_id);
    }
    
    return channel_id;
}

// Delete a channel
bool remove_channel(int channel_id) {
    // First, delete all entries from the channel_users table
    char query[200];
    sprintf(query, "DELETE FROM channel_users WHERE channel_id=%d", channel_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    PQclear(result);
    
    // Delete the channel itself
    sprintf(query, "DELETE FROM channels WHERE id=%d", channel_id);
    
    result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Check if a channel exists
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

// Get the channel name
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

// Get channel information
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
    
    // Compete channel structure
    channel->id = atoi(PQgetvalue(result, 0, 0));
    strcpy(channel->name, PQgetvalue(result, 0, 1));
    channel->is_private = strcmp(PQgetvalue(result, 0, 2), "t") == 0;
    channel->creator_id = atoi(PQgetvalue(result, 0, 3));
    
    PQclear(result);
    return true;
}

// Check if a user has authorisation to access a channel
bool can_access_channel(int user_id, int channel_id) {
    channel_t channel;
    if (!get_channel_by_id(channel_id, &channel)) {
        return false;
    }
    
    // If a channel is public, everyone has access
    if (!channel.is_private) {
        return true;
    }
    
    // If user is the creator, they have access
    if (channel.creator_id == user_id) {
        return true;
    }
    
    // Otherwise, check in channel_users table
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

// Check if a user is admin of a channel
bool is_channel_admin(int user_id, int channel_id) {
    // If a user is a global admin, they are admin everywhere
    if (is_admin(user_id)) {
        return true;
    }
    
    // Check if they created the channel
    channel_t channel;
    if (get_channel_by_id(channel_id, &channel) && channel.creator_id == user_id) {
        return true;
    }
    
    return false;
}

// Ajouter un utilisateur à un canal privé
bool add_user_to_channel(int user_id, int channel_id) {
    channel_t channel;
    
    // Check that channel exists and is private
    if (!get_channel_by_id(channel_id, &channel) || !channel.is_private) {
        return false;
    }
    
    // Check if user is already in the channel
    char query[200];
    sprintf(query, "SELECT user_id FROM channel_users WHERE channel_id=%d AND user_id=%d",
            channel_id, user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    // If user is already in the channel, no need to add them
    if (PQntuples(result) > 0) {
        PQclear(result);
        return true;
    }
    
    PQclear(result);
    
    // Add user to channel
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

// Remove a user from a private channel
bool remove_user_from_channel(int user_id, int channel_id) {
    channel_t channel;
    
    // Check is channel exists and is private
    if (!get_channel_by_id(channel_id, &channel) || !channel.is_private) {
        return false;
    }
    
    // Prevent removal of channel creator
    if (channel.creator_id == user_id) {
        return false;
    }
    
    // Delete user from channel
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

// Send history of messages of a channel to a user
void send_channel_history(int client_id, int channel_id) {
    // Get the 50 last messages
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
    
    // Send a header
    char channel_name[50];
    get_channel_name(channel_id, channel_name);
    
    char header[100];
    sprintf(header, "--- Channel history '%s' ---", channel_name);
    send_to_client(client_id, header);
    
    // Go through results from older to most recent
    int count = PQntuples(result);
    for (int i = count - 1; i >= 0; i--) {
        char message[BUFFER_SIZE];
        sprintf(message, "[%s] %s: %s", 
                PQgetvalue(result, i, 2),  // timestamp
                PQgetvalue(result, i, 1),  // username
                PQgetvalue(result, i, 0)); // content
        
        send_to_client(client_id, message);
    }
    
    send_to_client(client_id, "--- End of history ---");
    
    PQclear(result);
}