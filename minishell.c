/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_TOKENS 20             /* max number of command tokens */
#define INPUT_BUFFER_SIZE 100     /* input buffer size */
char line[INPUT_BUFFER_SIZE];     /* command input buffer */
char *SEPERATORS = " \t\n";

typedef enum {
    INPUT_VALID,
    INPUT_CONTINUE,  // skip this line, e.g. comment or empty
    INPUT_EXIT       // exit shell
} InputStatus;

#define MAX_JOBS 128
typedef struct {
    int id;
    pid_t process_id;
    int active;
    char cmd[INPUT_BUFFER_SIZE];
} Job;
static Job jobs[MAX_JOBS];
static int next_job_id = 1;

static int add_job(pid_t process_id, const char *cmd) {
    for (int index = 0; index < MAX_JOBS; index++)
    {
        if (!jobs[index].active)
        {
            jobs[index].active = 1;
            jobs[index].process_id = process_id;
            jobs[index].id = next_job_id++;
            snprintf(jobs[index].cmd, sizeof(jobs[index].cmd), "%s", cmd);
            return jobs[index].id;
        }
    }
    return -1;
}

static void tick_background(void) {
    int status;
    while (1==1)
    {
        pid_t process = waitpid(-1, &status, WNOHANG);
        if (process == 0) break;
        if (process == -1) {
            if (errno != ECHILD) perror("waitpid");
            break;
        }
        for (int index = 0; index < MAX_JOBS; index++)
        {
            if (jobs[index].active && jobs[index].process_id == process) {
                printf("[%d]+ Done                 %s\n", jobs[index].id, jobs[index].cmd);
                fflush(stdout);
                jobs[index].active = 0;
                break;
            }
        }
    }
}

static void build_cmd_string(char *tokens[], int count, char *out, size_t outsz){
    out[0] = '\0';
    for (int i=0; i<count; i++){
        if (tokens[i] == NULL) break;
        if (i) strncat(out, " ", outsz - strlen(out) - 1);
        strncat(out, tokens[i], outsz - strlen(out) - 1);
    }
}

InputStatus validate_input(char *line) {
    if (feof(stdin)) {  /* non-zero on EOF  */
        fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(), feof(stdin), ferror(stdin));
        return INPUT_EXIT;
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000'){
        return INPUT_CONTINUE;
    }
    return INPUT_VALID;
}

int capture_tokens(char *line, char *tokens[], int max_tokens, const char *separators) {
    if (line == NULL || tokens == NULL) return 0;

    tokens[0] = strtok(line, separators);
    if (tokens[0] == NULL) return 0;

    int count = 1;
    for (count = 1; count < max_tokens; count++) {
        tokens[count] = strtok(NULL, separators);
        if (tokens[count] == NULL) break;
    }
    return count;   // Number of parsed tokens
}

/* shell prompt */
void prompt(void) {
    // fprintf(stdout, "\n msh> ");
    // fflush(stdout);
}


static void change_directory(char *target) {
    const char *target_dir = target;

    if (!target_dir) {                         // cd
        target_dir = getenv("HOME");
        if (!target_dir) { fprintf(stderr, "cd: HOME not set\n"); return; }
    } else if (strcmp(target_dir, "~") == 0) { // cd ~
        target_dir = getenv("HOME");
        if (!target_dir) { fprintf(stderr, "cd: HOME not set\n"); return; }
    }

    char *old_cwd = getcwd(NULL, 0);

    if (chdir(target_dir) == -1) {
        perror("chdir");
        free(old_cwd);
        return;
    }

    if (old_cwd) {
        setenv("OLDPWD", old_cwd, 1);
        free(old_cwd);
    }
    char *new_cwd = getcwd(NULL, 0);
    if (new_cwd) {
        setenv("PWD", new_cwd, 1);
        free(new_cwd);
    }
}


// /* argk - number of arguments */
// /* argv - argument vector from command line */
// /* envp - environment pointer */
// int main(int argk, char *argv[], char *envp[])
//! all Unused so voiding atm until I see its used

int main(void) {
    int             fork_return_value;                // value returned by fork system call
    int             wait_return_value;                // value returned by wait system call
    char            *command_line_tokens[MAX_TOKENS];

    (void)wait_return_value;

    /* prompt for and process one command line at a time  */

    while (true) {    // do Forever
        tick_background();
        prompt();

        if (!fgets(line, INPUT_BUFFER_SIZE, stdin)) {
            if (feof(stdin)) exit(0);
            perror("fgets");
            continue;
        }

        // validate inputs
        switch (validate_input(line)) {
            case INPUT_EXIT:        exit(0);
            case INPUT_CONTINUE:    continue;
            case INPUT_VALID:       break;
        }

        // tokenize
        int token_count = capture_tokens(line, command_line_tokens, MAX_TOKENS, SEPERATORS);
        if (token_count == 0) continue;

        // cd managing
        if (strcmp(command_line_tokens[0], "cd") == 0) {
            if (command_line_tokens[1] && command_line_tokens[2]) {
                fprintf(stderr, "cd: too many arguments\n");
                continue;
            }
            change_directory(command_line_tokens[1]);
            continue;
        }


        bool background_execution = (token_count > 0 && strcmp(command_line_tokens[token_count - 1], "&") == 0);
        if (background_execution) {
            command_line_tokens[token_count - 1] = NULL;
        }

        // ensure argv is NULL-terminated for execvp
        if (!background_execution) command_line_tokens[token_count] = NULL;

        // build printable command string (no '&')
        char printable_cmd[INPUT_BUFFER_SIZE];
        build_cmd_string(command_line_tokens, token_count, printable_cmd, sizeof(printable_cmd));


        /* fork a child process to exec the command in command_line_tokens[0] */
        switch (fork_return_value = fork()) {
            case -1: {                  //! fork returns error to parent process
                perror("fork");
                break;
            }
            case 0: {                   //! code executed only by child process
                execvp(command_line_tokens[0], command_line_tokens);
                perror("execvp");
                _exit(127);
            }
            default: {                  //! code executed only by parent process
                if (background_execution) {
                    int job_id = add_job(fork_return_value, printable_cmd[0] ? printable_cmd : command_line_tokens[0]);
                    if (job_id < 1) {
                        fprintf(stderr, "job table full\n");
                    } else {
                        printf("[%d] %d\n", job_id, (int)fork_return_value);
                        fflush(stdout);
                    }
                    // no wait
                } else {
                    if (waitpid(fork_return_value, NULL, 0) == -1) perror("waitpid");
                }
                tick_background();
            }
        }
    }
}
