#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "user.h"
#include "database.h"
#include "crypto.h"

// Create a new user
bool create_user(user_t *user, const char *password) {
    // Check if email is valid
    if (!validate_email(user->email)) {
        printf("Error: Invalid email format\n");
        return false;
    }
    // Check if email already exists in database
    user_t existing_user;
    if (get_user_by_email(user->email, &existing_user)) {
        printf("Error: Email already in use\n");
        return false;
    }

      // Check if password is valid 
      if (!validate_password(password)) {
        printf("Error: Invalid password format\n");
        return false;
    }
    
    // Hash the password before saving
    char *hashed_password = hash_password(password);
    if (!hashed_password) {
        return false;
    }
    
    // User rights, by defaukt new users will be members
    user->role = 1;
    
    // Insert in the database
    char query[500];
    sprintf(query, 
            "INSERT INTO users (username, firstname, lastname, email, password, role) "
            "VALUES ('%s', '%s', '%s', '%s', '%s', %d) RETURNING id",
            user->username, user->firstname, user->lastname, 
            user->email, hashed_password, user->role);
    
    // Execute request and return generated ID
    PGresult *result = execute_query(query);
    
    // Free memory for the hashed password
    free(hashed_password);
    
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    // Get generated ID
    user->id = atoi(PQgetvalue(result, 0, 0));
    
    PQclear(result);
    return true;
}

// Update an existing user
bool update_user(user_t *user) {
    char query[500];
    sprintf(query, 
            "UPDATE users SET username='%s', firstname='%s', lastname='%s', "
            "email='%s', role=%d WHERE id=%d",
            user->username, user->firstname, user->lastname,
            user->email, user->role, user->id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Delete user
bool delete_user(int user_id) {
    char query[100];
    sprintf(query, "DELETE FROM users WHERE id=%d", user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Fidn a user via its ID
bool get_user_by_id(int user_id, user_t *user) {
    char query[100];
    sprintf(query, "SELECT * FROM users WHERE id=%d", user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Complete user information
    user->id = atoi(PQgetvalue(result, 0, 0));
    strcpy(user->username, PQgetvalue(result, 0, 1));
    strcpy(user->firstname, PQgetvalue(result, 0, 2));
    strcpy(user->lastname, PQgetvalue(result, 0, 3));
    strcpy(user->email, PQgetvalue(result, 0, 4));
    user->role = atoi(PQgetvalue(result, 0, 6));
    
    PQclear(result);
    return true;
}

// Find a user with its email address
bool get_user_by_email(const char *email, user_t *user) {
    char query[200];
    sprintf(query, "SELECT * FROM users WHERE email='%s'", email);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Complete user information
    user->id = atoi(PQgetvalue(result, 0, 0));
    strcpy(user->username, PQgetvalue(result, 0, 1));
    strcpy(user->firstname, PQgetvalue(result, 0, 2));
    strcpy(user->lastname, PQgetvalue(result, 0, 3));
    strcpy(user->email, PQgetvalue(result, 0, 4));
    user->role = atoi(PQgetvalue(result, 0, 6));
    
    PQclear(result);
    return true;
}

// Check sign-in information
bool verify_login(const char *email, const char *password, user_t *user) {
    // First check email
    if (!get_user_by_email(email, user)) {
        return false;
    }
    
    // Get corresponding user's encrypted password
    char query[200];
    sprintf(query, "SELECT password FROM users WHERE id=%d", user->id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return false;
    }
    
    // Get encrypted password
    char stored_hash[100];
    strcpy(stored_hash, PQgetvalue(result, 0, 0));
    
    PQclear(result);
    
    // Check password
    return verify_password(password, stored_hash);
}

// Function to validate an email address
bool validate_email(const char *email) {
    // Check if email contains @
    const char *at_sign = strchr(email, '@');
    if (!at_sign) return false;
    
    // Check if there is text after @
    if (at_sign == email || *(at_sign + 1) == '\0') return false;
    
    // Check if there is a dot after @
    const char *dot_after_at = strchr(at_sign, '.');
    if (!dot_after_at) return false;
    
    // Check if there is text after dot
    if (*(dot_after_at + 1) == '\0') return false;
    
    return true;
}

// Function to validate a password
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

// Check if user is an admin
bool is_admin(int user_id) {
    user_t user;
    if (!get_user_by_id(user_id, &user)) {
        return false;
    }
    
    return user.role == 3;
}

// Check if user is a moderator
bool is_moderator(int user_id) {
    user_t user;
    if (!get_user_by_id(user_id, &user)) {
        return false;
    }
    
    return user.role >= 2;
}

// Increase rights of a user
bool promote_user(int user_id, int new_role) {
    if (new_role < 1 || new_role > 3) {
        return false;
    }
    
    char query[100];
    sprintf(query, "UPDATE users SET role=%d WHERE id=%d", new_role, user_id);
    
    PGresult *result = execute_query(query);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        if (result) PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

// Function to encrypt a password
char* hash_password(const char *password) {
    return encrypt_password(password);
}