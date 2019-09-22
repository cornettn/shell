/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "shell.h"

/*
 *  Initialize a command_t
 */

void create_command(command_t *command) {
  command->single_commands = NULL;

  command->out_file = NULL;
  command->in_file = NULL;
  command->err_file = NULL;

  command->append_out = false;
  command->append_err = false;
  command->background = false;

  command->num_single_commands = 0;
} /* create_command() */

/*
 *  Insert a single command into the list of single commands in a command_t
 */

void insert_single_command(command_t *command, single_command_t *simp) {
  if (simp == NULL) {
    return;
  }

  command->num_single_commands++;
  int new_size = command->num_single_commands * sizeof(single_command_t *);
  command->single_commands = (single_command_t **)
                              realloc(command->single_commands,
                                      new_size);
  command->single_commands[command->num_single_commands - 1] = simp;
} /* insert_single_command() */

/*
 *  Free a command and its contents
 */

void free_command(command_t *command) {
  for (int i = 0; i < command->num_single_commands; i++) {
    free_single_command(command->single_commands[i]);
  }

  if (command->out_file) {
    free(command->out_file);
    command->out_file = NULL;
  }

  if (command->in_file) {
    free(command->in_file);
    command->in_file = NULL;
  }

  if (command->err_file) {
    free(command->err_file);
    command->err_file = NULL;
  }

  command->append_out = false;
  command->append_err = false;
  command->background = false;

  free(command);
} /* free_command() */

/*
 *  Print the contents of the command in a pretty way
 */

void print_command(command_t *command) {
  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   single Commands\n");
  printf("  --- ----------------------------------------------------------\n");

  // iterate over the single commands and print them nicely
  for (int i = 0; i < command->num_single_commands; i++) {
    printf("  %-3d ", i );
    print_single_command(command->single_commands[i]);
  }

  printf( "\n\n" );
  printf( "  Output       Input        Error        Background\n" );
  printf( "  ------------ ------------ ------------ ------------\n" );
  printf( "  %-12s %-12s %-12s %-12s\n",
            command->out_file?command->out_file:"default",
            command->in_file?command->in_file:"default",
            command->err_file?command->err_file:"default",
            command->background?"YES":"NO");
  printf( "\n\n" );
} /* print_command() */

/*
 *  Execute a command
 */

void execute_command(command_t *command) {
  // Don't do anything if there are no single commands
  if (command->single_commands == NULL) {
    print_prompt();
    return;
  }

  // Print contents of Command data structure
  print_command(command);

  // Add execution here

  /* Save standard in and out */

  int temp_in = dup(0);
  int temp_out = dup(1);

  /* Set input */

  int fd_in;
  if (command->in_file) {
    fd_in = open(command->in_file, O_RDWR|O_TRUNC);
    if (fd_in < 0) {
      perror("open");
      exit(1);
    }
  }
  else {

    /* Use the default standard in */

    fd_in = dup(temp_in);
  }


  int ret = -1;

  /* Create a new fork for each single command */

  for (int i = 0; i < command->num_single_commands; i++) {

    /* Redirect Input */

    dup2(fd_in, 0);
    close(fd_in);

    /* Setup Output*/

    if (i == command->num_single_commands - 1) {
      /* Last Single Command */
      if (command->out_file) {
        fd_out = open(command->out_file, O_CREAT|O_RDWR|O_TRUNC);
      }
      else {
        fd_out = dup(temp_out);
      }
    }
    else {
      /* Not the last Command - Use Pipes */
    }

    single_command_t * single_command = command->single_commands[i];
    ret = fork();
    if (ret == 0) {
      /* Child Process */


      /* Ensure that the last element in the arguments list is NULL */

      if (single_command->arguments[single_command->num_args - 1] != NULL) {
        single_command->arguments[single_command->num_args] = NULL;
      }

      execvp(single_command->arguments[0],
          single_command->arguments);

      /* execvp should never return on success, so if it does, error */

      perror("execvp");
      exit(1);
    }
    else if (ret < 0) {

      /* fork error */

      perror("fork");
      return;
    }

    /* Parent Process */

    if (!command->background) {
      waitpid(ret, NULL, 0);
    }

  }

  // Setup i/o redirection
  // and call exec

  // Clear to prepare for next command
  free_command(command);

  // Print new prompt
  print_prompt();
} /* execute_command() */
