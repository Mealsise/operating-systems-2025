#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

// Global flags set by signal handlers
volatile sig_atomic_t got_hup = 0;
volatile sig_atomic_t got_int = 0;

// Handle SIGHUP signal: sets flag to print "Ouch!"
void handle_hup(int sig) {got_hup = 1;}

// Handle SIGHUP signal: sets flag to print "Ouch!"
void handle_int(int sig) {got_int = 1;}

// Check signal flags and print messages when needed
void handle_flag(const char *msg, volatile sig_atomic_t *flag) {
    if (*flag) {
        printf("%s\n", msg);
        fflush(stdout);
        *flag = 0;
    }
}

int main (int argc, char *argv[]) {
    // Check usage
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of even integers>\n", argv[0]);
        return 1;
    }

    // Convert argument to int and validate
    int max_even = atoi(argv[1]);
    if (max_even < 0) {
        fprintf(stderr, "Please provide a non-negative integer.\n");
        return 1;
    }
    max_even <<= 1;

    // Register signal handlers
    signal(SIGHUP, handle_hup);
    signal(SIGINT, handle_int);

    // Loop through first n even numbers
    for (int to_print = 0; to_print <= max_even; to_print += 1) {
        printf("%d\n", to_print);
        fflush(stdout);

        // Only sleep if not the final number
        if (to_print < max_even) sleep(5);      // Does the if so that it doesn't wait after final print

        // Handle any received signals
        handle_flag("Ouch!", &got_hup);
        handle_flag("Yeah!", &got_int);
    }

    return 0;
}
