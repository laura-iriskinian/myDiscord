#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

// Prototypes for server functions
extern bool init_server(void);
extern void run_server(void);
extern void stop_server(void);

// File to store server PID
#define PID_FILE "/tmp/mydiscord_server.pid"

// Write PID in file to enable stopping server later
void write_pid_file() {
    FILE *file = fopen(PID_FILE, "w");
    if (file) {
        fprintf(file, "%d", getpid());
        fclose(file);
    }
}

// Delete PID file
void remove_pid_file() {
    unlink(PID_FILE);
}

// Manage signal to stop the server correctly
void handle_signal(int sig) {
    printf("\nSignal %d received, stopping server..\n", sig);
    remove_pid_file();
    stop_server();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Manage ways to stop server properly
    signal(SIGINT, handle_signal);   // Ctrl+C
    signal(SIGTERM, handle_signal);  // kill
    signal(SIGHUP, handle_signal);   // Closing terminal
    
    // Write PID in a file
    write_pid_file();
    
    printf("Starting MyDiscord server...\n");
    
    // Initialize server
    if (!init_server()) {
        fprintf(stderr, "Error when initializing server.\n");
        remove_pid_file();
        return EXIT_FAILURE;
    }
    
    // Run server (blocking function)
    run_server();
    
    // Stop server
    stop_server();
    remove_pid_file();
    
    return EXIT_SUCCESS;
}