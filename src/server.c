#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define PORT 21000

int clients[MAX_CLIENTS];            // tableau des sockets clients
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// 💬 Envoie un message à tous les clients sauf celui qui l'a envoyé
void broadcast(char *message, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_fd) {
            send(clients[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 🔁 Fonction exécutée dans un thread pour chaque client
void *handle_client(void *arg) {
    int client_fd = *((int *)arg);
    char buffer[1024];
    ssize_t read_size;

    while ((read_size = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Message reçu : %s", buffer);
        broadcast(buffer, client_fd);
    }

    // 🔌 Déconnexion
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_fd) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client_fd);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 🎯 Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 📍 Configuration de l’adresse serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    // 🔗 Liaison
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);

    printf("🟢 Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        printf("🔗 Nouveau client connecté : %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 🧩 Ajout du client
        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = client_fd;

            pthread_t tid;
            int *new_sock = malloc(sizeof(int));
            *new_sock = client_fd;
            pthread_create(&tid, NULL, handle_client, (void *)new_sock);
            pthread_detach(tid);
        } else {
            printf("❌ Trop de clients connectés !\n");
            close(client_fd);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
    return 0;
}
