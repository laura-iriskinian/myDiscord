#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "load_env.h"

// Variable globale pour la connexion à la base de données
static PGconn *db_connection = NULL;

// Initiate the connection to the database
bool init_database(void) {
    // Load environment variables from .env file 
    load_env("../../.env");

    const char *db_host = getenv("DB_HOST");
    const char *db_port = getenv("DB_PORT");
    const char *db_name = getenv("DB_NAME");
    const char *db_user = getenv("DB_USER");
    const char *db_password = getenv("DB_PASSWORD");

    if (!db_host || !db_port || !db_name || !db_user || !db_password) {
        fprintf(stderr, "Error: one or more environment variables are missing.\n");
        return false;
    }

    // Create connection
    char connection_string[512];
    snprintf(connection_string, sizeof(connection_string),
             "host=%s port=%s dbname=%s user=%s password=%s",
             db_host, db_port, db_name, db_user, db_password);

    // Connect
    db_connection = PQconnectdb(connection_string);

    if (PQstatus(db_connection) != CONNECTION_OK) {
        fprintf(stderr, "Error in the connection to database: %s\n",
                PQerrorMessage(db_connection));
        PQfinish(db_connection);
        db_connection = NULL;
        return false;
    }

    printf("Connection to database successfull!\n");
    printf("Connection string: %s\n", connection_string);

    return true;
}

// Close database
void close_database(void) {
    if (db_connection) {
        PQfinish(db_connection);
        db_connection = NULL;
    }
}

// Execute SQL request
PGresult *execute_query(const char *query) {
    if (!db_connection) {
        fprintf(stderr, "No connection to database\n");
        return NULL;
    }
    
    PGresult *result = PQexec(db_connection, query);
    
    if (!result) {
        fprintf(stderr, "Error in request execution: %s\n", 
                PQerrorMessage(db_connection));
        return NULL;
    }
    
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        fprintf(stderr, "Resquest failed: %s\n", PQerrorMessage(db_connection));
        fprintf(stderr, "Request: %s\n", query);
        PQclear(result);
        return NULL;
    }
    
    return result;
}

// Create tables if they don't exist
bool create_tables_if_not_exists(void) {
    // User tables
    const char *users_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "    id SERIAL PRIMARY KEY,"
        "    username VARCHAR(50) NOT NULL,"
        "    firstname VARCHAR(50) NOT NULL,"
        "    lastname VARCHAR(50) NOT NULL,"
        "    email VARCHAR(100) UNIQUE NOT NULL,"
        "    password VARCHAR(100) NOT NULL,"
        "    role INTEGER DEFAULT 1"  // 1=member, 2=moderator, 3=admin
        ");";
    
    // Channels table
    const char *channels_table = 
        "CREATE TABLE IF NOT EXISTS channels ("
        "    id SERIAL PRIMARY KEY,"
        "    name VARCHAR(50) NOT NULL,"
        "    is_private BOOLEAN DEFAULT FALSE,"
        "    creator_id INTEGER REFERENCES users(id)"
        ");";
    
    // Link table for private channels
    const char *channel_users_table = 
        "CREATE TABLE IF NOT EXISTS channel_users ("
        "    channel_id INTEGER REFERENCES channels(id) ON DELETE CASCADE,"
        "    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,"
        "    PRIMARY KEY (channel_id, user_id)"
        ");";
    
    // Messages table
    const char *messages_table = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "    id SERIAL PRIMARY KEY,"
        "    user_id INTEGER REFERENCES users(id),"
        "    channel_id INTEGER REFERENCES channels(id) ON DELETE CASCADE,"
        "    content TEXT NOT NULL,"
        "    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    is_encrypted BOOLEAN DEFAULT FALSE"
        ");";
    
    // Emojis table :-)
    const char *reactions_table = 
        "CREATE TABLE IF NOT EXISTS reactions ("
        "    id SERIAL PRIMARY KEY,"
        "    message_id INTEGER REFERENCES messages(id) ON DELETE CASCADE,"
        "    user_id INTEGER REFERENCES users(id),"
        "    emoji VARCHAR(10) NOT NULL,"
        "    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    // Create all tables
    PGresult *result;
    
    result = execute_query(users_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(channels_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(channel_users_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(messages_table);
    if (!result) return false;
    PQclear(result);
    
    result = execute_query(reactions_table);
    if (!result) return false;
    PQclear(result);
    
    // Check if the main channel already exists
    result = execute_query("SELECT id FROM channels WHERE id=1");
    if (!result) return false;
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        
        // Create main channel (public)
        result = execute_query(
            "INSERT INTO channels (id, name, is_private, creator_id) "
            "VALUES (1, 'Général', FALSE, NULL)"
        );
        
        if (!result) return false;
        PQclear(result);
    } else {
        PQclear(result);
    }
    
    // Check if default admin exists
    result = execute_query("SELECT id FROM users WHERE email='admin@mydiscord.com'");
    if (!result) return false;
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        
        // Create default admin (password: admin123)
        result = execute_query(
            "INSERT INTO users (username, firstname, lastname, email, password, role) "
            "VALUES ('Admin', 'Admin', 'MyDiscord', 'admin@mydiscord.com', 'hashed_admin123', 3)"
        );
        
        if (!result) return false;
        PQclear(result);
    } else {
        PQclear(result);
    }
    
    return true;
}