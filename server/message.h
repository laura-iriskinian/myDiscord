#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>
#include <time.h>

// Structure to represent a message
typedef struct {
    int id;             // ID in database
    int user_id;        // ID of sender
    int channel_id;     // channel ID
    char content[1024]; // Message content
    time_t timestamp;   // Timestamp
    bool is_encrypted;  // If message is encrypted
} message_t;

// Functions to handle messages
bool save_message(int user_id, int channel_id, const char *content);
bool get_message_by_id(int message_id, message_t *message);
bool delete_message(int message_id, int user_id);
bool add_reaction(int message_id, int user_id, const char *emoji);
bool remove_reaction(int message_id, int user_id, const char *emoji);

#endif