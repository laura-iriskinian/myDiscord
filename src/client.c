#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 21000
#define SERVER_IP "127.0.0.1" // Server IP address

int sockfd;

void *send_message(void *arg) {
    char sendline[100];

    while (1) {
        printf("Your message: ");
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
            printf("Connection error or server disconnected.\n");
            break;
        }
        recvline[n] = '\0';
        printf(" %s", recvline);
    }

    return NULL;
}

int main() {
    struct sockaddr_in servaddr;

    // Creates socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error while creating socket");
        exit(1);
    }

    // Configure server's IP
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(1);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // Create threads
    pthread_t send_thread, recv_thread;

    // Create thread to send messages
    if (pthread_create(&send_thread, NULL, send_message, NULL) != 0) {
        perror("Error during creation of sending thread");
        close(sockfd);
        exit(1);
    }

    // Create thread to receive messages
    if (pthread_create(&recv_thread, NULL, receive_message, NULL) != 0) {
        perror("Error during creation of receiving thread");
        close(sockfd);
        exit(1);
    }

    // Wait until the threads finish 
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);
    return 0;
}
