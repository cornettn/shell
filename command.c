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
#include <fcntl.h>

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

  print_command(command);

  /* Save standard in, err and out */

  int default_in = dup(0);
  int default_out = dup(1);
  int deafult_err = dup(2);

  /* Set input */

  int fd_in;
  if (command->in_file) {
    fd_in = open(command->in_file, O_CREAT|O_RDONLY, 0400);
    printf("fd_in is in file\n");
    if (fd_in < 0) {
      perror("open");
      exit(1);
    }
  }
  else {
    printf("fd_in is default\n");
    fd_in = dup(default_in);
  }


  /* Setup Error output */

  int fd_err;
  if (command->err_file) {
    if (command->append_err) {
      fd_err = open(command->err_file, O_CREAT|O_RDWR|O_APPEND, 0600);
    }
    else {
      fd_err = open(command->err_file, O_CREAT|O_RDWR|O_TRUNC, 0600);
    }

    if (fd_err < 0) {
      perror("open");
      exit(1);
    }
  }
  else {
    fd_err = dup(default_err);
  }


  int ret = -1;
  int fd_pipe[2];
  bool read_pipe = false;
  bool write_pipe = false;

  /* Create a new fork for each single command */

  for (int i = 0; i < command->num_single_commands; i++) {
    printf("next single command\n");

    /* Redirect Input to fd_in if input is not coming from pipes */

    if (!read_pipe) {
      printf("Redirect input to fd_in\n");
      dup2(fd_in, 0);
      close(fd_in);
    }
    else {
      printf("Close fd_in\n");
      close(fd_in);  // Not using fd_in anymore
      printf("Redirect input to fd_pipe[0]\n");
      dup2(fd_pipe[0], 0);
      printf("Close fd_pipe[0]\n");
      close(fd_pipe[0]);
      read_pipe = false;
    }

    /* Setup Output*/

    int fd_out;
    if (i == command->num_single_commands - 1) {

      /* Last Single Command */

      /* Output */
      if (command->out_file) {
        printf("fd_out is out file\n");
        if (command->append_out) {
          fd_out = open(command->out_file,
              O_CREAT|O_APPEND|O_RDWR, 0600);
        }
        else {
          fd_out = open(command->out_file, O_CREAT|O_RDWR|O_TRUNC, 0600);
        }
      }
      else {
        printf("fd_out to default\n");
        fd_out = dup(default_out);
      }
    }
    else {
      /* Not the last Command - Use Pipes */

      printf("Use pipes\n");

      if (pipe(fd_pipe) == -1) {
        perror("pipe");
      }

      /* Set Flags */

      write_pipe = true;
      read_pipe = true;
    }

//    printf("Redirect input out and err\n");


    /* Redirect Error */

    dup2(fd_err, 2);
    close(fd_err);

    /* Redirect Output */

    if (write_pipe) {
      printf("Close fd_out\n");
      close(fd_out); // Not using it
      write_pipe = false;
      printf("Redirect output to fd_pipe[1]\n");
      dup2(fd_pipe[1], 1);
      printf("Close fd_pipe[1]\n");
      close(fd_pipe[1]);
    }
    else {
      printf("Write to fd_out\n");
      dup2(fd_out, 1);
      close(fd_out);
    }

    /* Create a child process */

//    printf("Fork\n");

    single_command_t * single_command = command->single_commands[i];
    ret = fork();
    if (ret == 0) {
      /* Child Process */
      /* Ensure that the last element in the arguments list is NULL */

      if (single_command->arguments[single_command->num_args - 1] != NULL) {
        single_command->arguments = (char **) realloc(single_command->arguments,
            (single_command->num_args + 1) * sizeof(char *));
        single_command->arguments[single_command->num_args] = NULL;
      }

//      printf("Execute Command\n");

      close(fd_pipe[0]);
      close(fd_pipe[1]);
      close(fd_in);
      close(fd_out);
      close(fd_err);
//      close(default_in);
//      close(default_out);
//      close(default_err);

      execvp(single_command->arguments[0],
          single_command->arguments);

      /* execvp should never return on success, so if it does, error */

      perror("execvp");
      exit(1);
    }
    else if (ret < 0) {

      /* fork error */
//    printf("Fork Error\n");

      perror("fork");
      return;
    }

    /* Parent Process */

//    printf("Parent\n");

    /* Restore in/out defaults */

    printf("Redirect input to default\n");
    dup2(deafult_in, 0);
    printf("Redirect output to default\n");
    dup2(default_out, 1);
    printf("Redirect err to default\n");
    dup2(default_err, 2);
    printf("Close defaults\n");
    close(default_in);
    close(default_err);
    close(default_out);
    printf("Close fd_in, fd_out, and fd_err\n");
    close(fd_in);
    close(fd_out);
    close(fd_err);

    if (!command->background) {
//      printf("Waiting for child\n");
      waitpid(ret, 0, 0);
//      printf("Done waiting\n");
    }


  } // End for loop

  // Setup i/o redirection
  // and call exec

  // Clear to prepare for next command
//  printf("Free this command\n");
//  print_command(command);

  free_command(command);

  // Print new prompti

//  printf("Print Prompt\n");
  print_prompt();
} /* execute_command() */
