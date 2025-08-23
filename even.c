#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

// async-signal-safe writer
static void say(const char *str) {
    while (*str)
    {
        ssize_t voided = write(STDOUT_FILENO, str, 1);
        (void)voided;
        str++;
    }
}

static void handle_hup(int sig) {
    (void)sig;
    say("Ouch!\n");
}

static void handle_int(int sig) {
    (void)sig;
    say("Yeah!\n");
}

int main (int argc, char *argv[]) {
    // Check usage
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of even integers>\n", argv[0]);
        return 1;
    }

    // Convert argument to int and validate
    int count = atoi(argv[1]);
    if (count < 0) {
        fprintf(stderr, "Please provide a non-negative integer.\n");
        return 1;
    }
    int max_even = (count<<1) - 1;

    // Register signal handlers
    struct sigaction sig_action_hup;
    struct sigaction sig_action_int;

    sigemptyset(&sig_action_hup.sa_mask);
    sig_action_hup.sa_flags = 0;                  /* let sleep be interrupted */
    sig_action_hup.sa_handler = handle_hup;
    if (sigaction(SIGHUP, &sig_action_hup, NULL) == -1){ perror("sigaction SIGHUP"); return 1; }

    sigemptyset(&sig_action_int.sa_mask);
    sig_action_int.sa_flags = 0;
    sig_action_int.sa_handler = handle_int;
    if (sigaction(SIGINT, &sig_action_int, NULL) == -1){ perror("sigaction SIGINT"); return 1; }



    // Loop through first n even numbers
    for (int to_print = 0; to_print <= max_even; to_print += 2) {
        printf("%d\n", to_print);
        fflush(stdout);

        // Only sleep if not the final number
        if (to_print < max_even) sleep(5);      // Does the if so that it doesn't wait after final print
    }

    return 0;
}
