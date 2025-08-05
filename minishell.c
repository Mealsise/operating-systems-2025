/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_TOKENS 20             /* max number of command tokens */
#define INPUT_BUFFER_SIZE 100     /* input buffer size */
char line[INPUT_BUFFER_SIZE];     /* command input buffer */
char *SEPERATORS = " \t\n";

/* shell prompt */
void prompt(void) {
  fprintf(stdout, "\n msh> ");
  fflush(stdout);
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

  /* prompt for and process one command line at a time  */

  while (true) {    // do Forever
    prompt();
    fgets(line, INPUT_BUFFER_SIZE, stdin);
    fflush(stdin);

    if (feof(stdin)) {  /* non-zero on EOF  */
      fprintf(
              stderr,
              "EOF pid %d feof %d ferror %d\n",
              getpid(),
      	      feof(stdin),
              ferror(stdin));
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000'){
      continue;			/* to prompt */
    }

    command_line_tokens[0] = strtok(line, SEPERATORS);

    for (int parse_index = 1; parse_index < MAX_TOKENS; parse_index++) {
      command_line_tokens[parse_index] = strtok(NULL, SEPERATORS);
      if (command_line_tokens[parse_index] == NULL){
	      break;
      }
    }
    /* assert parse_index is number of tokens + 1 */

    /* fork a child process to exec the command in command_line_tokens[0] */
    switch (fork_return_value = fork()) {
      case -1: { break; }     /* fork returns error to parent process */
      case 0: {               /* code executed only by child process */
	      execvp(command_line_tokens[0], command_line_tokens);
      }
      default: {              /* code executed only by parent process */
      	wait_return_value = wait(0);
        printf("%s done \n", command_line_tokens[0]);
      }
    }   /* switch */
  }     /* while */
}       /* main */
