#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "server.h"

// Signal manager to stop server properly
void handle_signal(int sig) {
    printf("\nSignal %d received, server stopping...\n", sig);
    stop_server();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Manage signal to stop the server properly
    signal(SIGINT, handle_signal);   // Ctrl+C
    signal(SIGTERM, handle_signal);  // kill
    
    printf("Server MyDiscord starting...\n");
    
    // Initialize the server
    if (!init_server()) {
        fprintf(stderr, "Error in server initialization\n");
        return EXIT_FAILURE;
    }
    
    // Run server (this function is blocking)
    run_server();
    
    // top the server (this should never be used here)
    stop_server();
    
    return EXIT_SUCCESS;
}