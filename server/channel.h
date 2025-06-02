#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>

// Structure to represent a channel
typedef struct {
    int id;                 // ID in database
    char name[50];          // Name of channel
    bool is_private;        // Private or public channel
    int creator_id;         // ID of user who created the channel
} channel_t;

// Functions to manage channels
int add_channel(const char *name, bool is_private, int creator_id);
bool remove_channel(int channel_id);
bool channel_exists(int channel_id);
bool get_channel_name(int channel_id, char *name);
bool get_channel_by_id(int channel_id, channel_t *channel);

// Handle authorisations
bool can_access_channel(int user_id, int channel_id);
bool is_channel_admin(int user_id, int channel_id);
bool add_user_to_channel(int user_id, int channel_id);
bool remove_user_from_channel(int user_id, int channel_id);

// Get messages
void send_channel_history(int client_id, int channel_id);

#endif 