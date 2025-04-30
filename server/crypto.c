#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypto.h"

// Clé de chiffrement simple (à remplacer par une meilleure solution en production)
static const char *SECRET_KEY = "MyDiscordSecretKey";

// Fonction simple de hachage pour les mots de passe
// Note: Dans une application réelle, utiliser bcrypt, Argon2 ou PBKDF2
char* encrypt_password(const char *password) {
    if (!password) return NULL;
    
    size_t passlen = strlen(password);
    size_t keylen = strlen(SECRET_KEY);
    char *hashed = (char*)malloc(passlen + 1);
    
    if (!hashed) return NULL;
    
    // XOR simple avec la clé secrète
    for (size_t i = 0; i < passlen; i++) {
        hashed[i] = password[i] ^ SECRET_KEY[i % keylen];
    }
    
    hashed[passlen] = '\0';
    
    // Convertir en hexadécimal pour le stockage
    char *hex = (char*)malloc(passlen * 2 + 1);
    if (!hex) {
        free(hashed);
        return NULL;
    }
    
    for (size_t i = 0; i < passlen; i++) {
        sprintf(hex + i * 2, "%02x", (unsigned char)hashed[i]);
    }
    
    free(hashed);
    return hex;
}

// Vérifier un mot de passe
bool verify_password(const char *password, const char *hashed) {
    if (!password || !hashed) return false;
    
    char *test_hash = encrypt_password(password);
    if (!test_hash) return false;
    
    bool result = (strcmp(test_hash, hashed) == 0);
    free(test_hash);
    
    return result;
}

// Chiffrer un message
char* encrypt_message(const char *message) {
    if (!message) return NULL;
    
    size_t msglen = strlen(message);
    size_t keylen = strlen(SECRET_KEY);
    char *encrypted = (char*)malloc(msglen + 1);
    
    if (!encrypted) return NULL;
    
    // XOR simple avec la clé secrète
    for (size_t i = 0; i < msglen; i++) {
        encrypted[i] = message[i] ^ SECRET_KEY[i % keylen];
    }
    
    encrypted[msglen] = '\0';
    
    // Convertir en hexadécimal pour le stockage
    char *hex = (char*)malloc(msglen * 2 + 1);
    if (!hex) {
        free(encrypted);
        return NULL;
    }
    
    for (size_t i = 0; i < msglen; i++) {
        sprintf(hex + i * 2, "%02x", (unsigned char)encrypted[i]);
    }
    
    free(encrypted);
    return hex;
}

// Déchiffrer un message
char* decrypt_message(const char *encrypted) {
    if (!encrypted) return NULL;
    
    size_t hexlen = strlen(encrypted);
    if (hexlen % 2 != 0) return NULL;
    
    size_t msglen = hexlen / 2;
    size_t keylen = strlen(SECRET_KEY);
    
    // Convertir l'hexadécimal en binaire
    unsigned char *binary = (unsigned char*)malloc(msglen);
    if (!binary) return NULL;
    
    for (size_t i = 0; i < msglen; i++) {
        sscanf(encrypted + i * 2, "%2hhx", &binary[i]);
    }
    
    // Déchiffrer avec XOR
    char *decrypted = (char*)malloc(msglen + 1);
    if (!decrypted) {
        free(binary);
        return NULL;
    }
    
    for (size_t i = 0; i < msglen; i++) {
        decrypted[i] = binary[i] ^ SECRET_KEY[i % keylen];
    }
    
    decrypted[msglen] = '\0';
    free(binary);
    
    return decrypted;
}