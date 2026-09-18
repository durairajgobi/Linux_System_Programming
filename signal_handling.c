#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile int signal_count = 0;
volatile int should_exit = 0;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    signal_count++;
    printf("\nReceived SIGINT (%d times)\n", signal_count);
    if (signal_count >= 3) {
        printf("Exiting after 3 interrupts\n");
        should_exit = 1;
    }
}

// Signal handler for SIGUSR1
void sigusr1_handler(int sig) {
    printf("Received SIGUSR1 - User-defined signal\n");
}

// Signal handler for SIGTERM
void sigterm_handler(int sig) {
    printf("Received SIGTERM - Terminating gracefully\n");
    should_exit = 1;
}

int main() {
    // Example 1: Basic signal registration
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);
    
    // Example 2: Register multiple signal handlers
    sa.sa_handler = sigusr1_handler;
    sigaction(SIGUSR1, &sa, NULL);
    
    sa.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &sa, NULL);
    
    printf("Signal handling example started (PID: %d)\n", getpid());
    printf("Try: kill -USR1 %d\n", getpid());
    printf("Try: kill -TERM %d\n", getpid());
    printf("Or press Ctrl+C three times\n");
    
    // Main loop - catches signals
    int counter = 0;
    while (!should_exit) {
        sleep(1);
        counter++;
        printf("Working... [%d seconds]\n", counter);
    }
    
    printf("Program exiting\n");
    return 0;
}
