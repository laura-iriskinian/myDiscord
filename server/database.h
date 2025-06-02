#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <libpq-fe.h>
#include "load_env.h"

// Functions to manage the database
bool init_database(void);
void close_database(void);
PGresult *execute_query(const char *query);
bool create_tables_if_not_exists(void);

#endif 