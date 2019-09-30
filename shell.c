#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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


void sig_int_handler() {
  if (isatty(0)) {
    printf("\n");
    print_prompt();
  }
}

/*
void sig_child_handler(int sid) {
  if (g_current_command->background) {
    printf("[%d] exited.\n", sid);
    print_prompt();
  }
}
*/

/*
 *  This main is simply an entry point for the program which sets up
 *  memory for the rest of the program and the turns control over to
 *  yyparse and never returns
 */

int main() {
  g_current_command = (command_t *)malloc(sizeof(command_t));
  g_current_single_command =
        (single_command_t *)malloc(sizeof(single_command_t));

  create_command(g_current_command);
  create_single_command(g_current_single_command);

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
} /* main() */
