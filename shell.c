#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "read_line.h"
#include "command.h"
#include "single_command.h"

command_t *g_current_command = NULL;
single_command_t *g_current_single_command = NULL;

int yyparse(void);

/*
 *  Prints shell prompt
 */

void print_prompt() {
  printf("myshell>");
  fflush(stdout);
} /* print_prompt() */


/*
 * This function is responsible for handling SIGCHILD.
 */

void sig_int_handler() {
  g_line_buffer[0] = 0;
  g_line_length = 0;
  if (isatty(0)) {
    printf("\n");
    print_prompt();
  }
} /* sig_int_handler()  */

/*
 *  This main is simply an entry point for the program which sets up
 *  memory for the rest of the program and the turns control over to
 *  yyparse and never returns
 */

int main(int argc, char **argv) {
  argc = 0;
  g_shell_name = argv[0];

  g_current_command = (command_t *)malloc(sizeof(command_t));

  create_command(g_current_command);

  if (isatty(0)) {
    print_prompt();
  }

  /* Signal Handling */
  struct sigaction signal_action;
  signal_action.sa_handler = sig_int_handler;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = SA_RESTART;
  int error = sigaction(SIGINT, &signal_action, NULL);

  if (error) {
    perror("sigaction");
    exit(2);
  }

  yyparse();
  return argc;
} /* main() */
