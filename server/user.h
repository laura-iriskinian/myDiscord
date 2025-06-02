#ifndef USER_H
#define USER_H

#include <stdbool.h>

// Structure representing a user
typedef struct {
    int id;                 
    char username[50];      
    char firstname[50];     
    char lastname[50];      
    char email[100];        
    int role;               // Role: 1=member, 2=moderator, 3=admin
} user_t;

// Functions to manage user
bool create_user(user_t *user, const char *password);
bool update_user(user_t *user);
bool delete_user(int user_id);
bool get_user_by_id(int user_id, user_t *user);
bool get_user_by_email(const char *email, user_t *user);
bool verify_login(const char *email, const char *password, user_t *user);

// Functions for roles and rights
bool is_admin(int user_id);
bool is_moderator(int user_id);
bool promote_user(int user_id, int new_role);

// Functions to validate
bool validate_email(const char *email);
bool validate_password(const char *password);

// Functions to secure password
char* hash_password(const char *password);

#endif 