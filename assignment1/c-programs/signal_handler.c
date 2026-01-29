#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Signal handler function
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Program will continue.\n");
    } 
    else if (sig == SIGUSR1) {
        printf("Caught SIGUSR1 (user-defined signal).\n");
    }
}

int main() {
    // Register signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGUSR1, handle_signal);

    printf("Signal handler running.\n");
    printf("PID: %d\n", getpid());
    printf("Press Ctrl+C or send SIGUSR1 using kill.\n");

    // Keep program alive
    while (1) {
        pause(); // Wait for signals
    }

    return 0;
}
