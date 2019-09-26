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

void sig_child_handler(int sig) {
  waitpid(sig, NULL, 0);
}

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
  int debug = open("debug", O_CREAT|O_TRUNC|O_RDWR, 0666);

  /* Don't do anything if there are no single commands */

  if (command->single_commands == NULL) {
    int input = dup(0);
    if (isatty(input)) {
      print_prompt();
    }
    close(input);
    return;
  }

//  print_command(command);

  /* Save default file descriptors */

  int default_in = dup(0);
  int default_out = dup(1);
  int default_err = dup(2);

  dprintf(debug, "Default in out err: %d %d %d\n", default_in, default_out, default_err);
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

  /* Redirect Error */

  dup2(fd_err, 2);
  close(fd_err);

  /* Set intial input */

  int fd_in;
  if (command->in_file) {
    fd_in = open(command->in_file, O_CREAT|O_RDONLY, 0400);
    if (fd_in < 0) {
      perror("open");
      exit(1);
    }
  }
  else {
    fd_in = dup(default_in);
  }

  /* Initialize variables to use inside the loop */

  int ret = -1;
  int fd_out;

  /* Create a new fork for each single command */
dprintf(debug, "Num single commands: %d\n", command->num_single_commands);
  for (int i = 0; i < command->num_single_commands; i++) {

    /* Redirect Input */

    dprintf(debug, "Redirect in\n");
    dprintf(debug, "fd_in: %d\n\n", fd_in);
    dup2(fd_in, 0);
    close(fd_in);
    dprintf(debug, "After close\n");
    dprintf(debug, "fd_in: %d\n\n", fd_in);


    //    fd_in = -1;
    /* Setup Output*/

    if (i == command->num_single_commands - 1) {

      /* Last Single Command */

      //print_single_command(command->single_commands[i]);

      if (command->out_file) {
        if (command->append_out) {
          fd_out = open(command->out_file,
              O_CREAT|O_APPEND|O_RDWR, 0600);
        }
        else {
          fd_out = open(command->out_file, O_CREAT|O_RDWR|O_TRUNC, 0600);
        }
      }
      else {
        fd_out = dup(default_out);
      }
    dprintf(debug, "In Last command: fd_out is %d\n\n", fd_out);
    }
    else {

      /* Not the last Command - Use Pipes */

      int fd_pipe[2];
      if (pipe(fd_pipe) == -1) {
        perror("pipe");
        return;
      }

      /* Make the next funtion read from pipe */

      fd_in = fd_pipe[0];

      /* Make the current function output to pipe */
      fd_out = fd_pipe[1];

      dprintf(debug, "Pipes: fd_out is %d, fd_in is %d\n\n", fd_out, fd_in);
    }

    /* Redirect Output */

    dprintf(debug, "Redirect out: fd_out is %d\n", fd_out);
    dup2(fd_out, 1);
    close(fd_out);

    /* Create a child process */
    single_command_t * single_command = command->single_commands[i];
    ret = fork();
    if (ret == 0) {
      dprintf(debug, "child\n");
      /* Ensure that the last element in the arguments list is NULL */

      if (single_command->arguments[single_command->num_args - 1] != NULL) {
        single_command->arguments = (char **) realloc(single_command->arguments,
            (single_command->num_args + 1) * sizeof(char *));
        single_command->arguments[single_command->num_args] = NULL;
      }

      /* Close defaults - child does not need them */

      close(default_in);
      close(default_out);
      close(default_err);

      dprintf(debug, "Within Child:\n\tfd_out: %d\n\tfd_in: %d\n", fd_out, fd_in);

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
    else {
      dprintf(debug, "Wait for %d\n", ret);
//      waitpid(ret, NULL, 0);
    }
  } // End for loop

  /* Parent Process */

  close(fd_in);

  /* Restore in/out defaults */

  dup2(default_in, 0);
  dup2(default_out, 1);
  dup2(default_err, 2);
  close(default_in);
  close(default_err);
  close(default_out);

  if (!command->background) {
    //printf("Waiting\n");
    waitpid(ret, NULL, 0);
    //printf("Done Waiting\n");
  }
  else {
    struct sigaction sa_zombies;
    sa_zombies.sa_handler = sig_child_handler;
    sigemptyset(&sa_zombies.sa_mask);
    sa_zombies.sa_flags = SA_RESTART|SA_NOCLDSTOP|SA_NOCLDWAIT;
    int zombie = sigaction(SIGCHLD, &sa_zombies, NULL);

    if (zombie) {
      perror("sigaction");
      exit(2);
     }
  }

  free_command(command);

  if (isatty(0)) {
    print_prompt();
  }
} /* execute_command() */
