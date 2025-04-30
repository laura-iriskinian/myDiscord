#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdbool.h>

// Fonctions pour le chiffrement et déchiffrement
char* encrypt_password(const char *password);
bool verify_password(const char *password, const char *hashed);
char* encrypt_message(const char *message);
char* decrypt_message(const char *encrypted);

#endif 