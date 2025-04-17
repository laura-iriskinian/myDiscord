#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 21000
#define SERVER_IP "127.0.0.1" // Adresse du serveur

int sockfd;

void *send_message(void *arg) {
    char sendline[100];

    while (1) {
        printf("Votre message: ");
        fgets(sendline, sizeof(sendline), stdin);
        send(sockfd, sendline, strlen(sendline), 0);
    }

    return NULL;
}

void *receive_message(void *arg) {
    char recvline[100];

    while (1) {
        ssize_t n = recv(sockfd, recvline, sizeof(recvline) - 1, 0);
        if (n <= 0) {
            printf("Erreur de connexion ou serveur déconnecté.\n");
            break;
        }
        recvline[n] = '\0';
        printf("Message du serveur: %s", recvline);
    }

    return NULL;
}

int main() {
    struct sockaddr_in servaddr;

    // Création du socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connexion échouée");
        close(sockfd);
        exit(1);
    }

    printf("Connecté au serveur %s:%d\n", SERVER_IP, PORT);

    // Création des threads
    pthread_t send_thread, recv_thread;

    // Création du thread pour envoyer les messages
    if (pthread_create(&send_thread, NULL, send_message, NULL) != 0) {
        perror("Erreur lors de la création du thread d'envoi");
        close(sockfd);
        exit(1);
    }

    // Création du thread pour recevoir les messages
    if (pthread_create(&recv_thread, NULL, receive_message, NULL) != 0) {
        perror("Erreur lors de la création du thread de réception");
        close(sockfd);
        exit(1);
    }

    // Attente que les threads finissent
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);
    return 0;
}
