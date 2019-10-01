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
#include <string.h>
#include <errno.h>

#include "shell.h"

bool background = false;
int g_debug;

void sig_child_handler(int sid) { //, siginfo_t *info, void *ucontext) {
  if (background) {
    printf("[%d] exited.\n", sid);
    print_prompt();
  }
}

void change_directory(char *dir) {
  if ((*dir) == '/') {

    /* Passed an absolute path */

    chdir(dir);
  }
  else {

    /* Not an absolute path */

    /* Get the cwd */

    size_t size = strlen(getenv("PWD")) * sizeof(char);
    char *cwd = (char *) malloc(size);
    char *result = getcwd(cwd, size);
    while (result == NULL) {
      size *= 2;
      free(cwd);
      cwd = (char *) malloc(size);
      result = getcwd(cwd, size);
    }

    int cwd_len = strlen(cwd);
    int dir_len = strlen(dir);

    char *copy = strdup(cwd);

    free(cwd);
    cwd = (char *)malloc(cwd_len + dir_len + 1);
    strcpy(cwd, copy);
    free(copy);

    *(cwd + cwd_len) = '/';
    *(cwd + cwd_len + 1) = '\0';
    cwd_len = strlen(cwd);

    for (size_t i = 0; i < strlen(dir); i++) {
      *(cwd + cwd_len + i) = *(dir + i);
    }
    //*(cwd + cwd_len + strlen(dir)) = '\0';

    if (chdir(cwd) < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "cd: can't cd to %s\n", dir);
      }
    }
    free(cwd);
  }
}

int execute_builtin(single_command_t *single) {
  dprintf(g_debug, "arg: \"%s\"\n", single->arguments[0]);

  if (!strcmp(single->arguments[0], "exit")) {
    dprintf(g_debug, "Def should exit\n");
    free_command(g_current_command);
    exit(1);
  }
  else if (!strcmp(single->arguments[0], "printenv")) {
    char **var = __environ;
    while (*var != NULL) {
      printf("%s\n", (*var));
      var++;
    }
    return 1;
  }
  else if (!strcmp(single->arguments[0], "setenv")) {
    setenv(single->arguments[1], single->arguments[2], 1);
    return 1;
  }
  else if (!strcmp(single->arguments[0], "unsetenv")) {
    unsetenv(single->arguments[1]);
    return 1;
  }
  else if (!strcmp(single->arguments[0], "cd")) {
    if (single->arguments[1]) {
      change_directory(single->arguments[1]);
    }
    else {
      chdir(getenv("HOME"));
    }
    return 1;
  }

  return 0;
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

  free(command->single_commands);
  command->single_commands = NULL;

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
  command = NULL;
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
 * This function will set standard error according to the specifications.
 */

int set_fd_err(command_t *command, int default_err) {
  int fd_err = -1;
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
  return fd_err;
} /* set_fd_err() */

/*
 *  Execute a command
 */

void execute_command(command_t *command) {

  g_debug = open("debug", O_CREAT||O_RDWR|O_APPEND, 0600);

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

  /* Setup Error output */

  int fd_err = set_fd_err(command, default_err);

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

  for (int i = 0; i < command->num_single_commands; i++) {
    single_command_t * single_command = command->single_commands[i];

    /* Redirect Input */

    dup2(fd_in, 0);
    close(fd_in);

    /* Setup Output*/

    if (i == command->num_single_commands - 1) {

      /* Last Single Command */

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

    }

    /* Redirect Output */

    dup2(fd_out, 1);
    close(fd_out);

    /* Create a child process */

    int builtin = execute_builtin(single_command);
    dprintf(g_debug, "Bultin: %d\n", builtin);

    ret = fork();
    if (ret == 0) {


      /* Ensure that the last element in the arguments list is NULL */

      if (single_command->arguments[single_command->num_args - 1] != NULL) {
        single_command->arguments = (char **) realloc(single_command->arguments,
            (single_command->num_args + 1) * sizeof(char *));
        single_command->arguments[single_command->num_args] = NULL;
        single_command->num_args++;
      }

      /* Close defaults - child does not need them */

      close(default_in);
      close(default_out);
      close(default_err);

      dprintf(g_debug, "Builtin: %d\n", builtin);
      if (builtin != 1) {
        execvp(single_command->arguments[0],
          single_command->arguments);
      }

      /* execvp should never return on success, so if it does, error */

      perror("execvp");
      exit(1);
    }
    else if (ret < 0) {

      /* fork error */

      perror("fork");
      return;
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

  /* Set up signal handling for SIGCHLD */

  struct sigaction sa_zombies;
  sa_zombies.sa_handler = sig_child_handler;
  sigemptyset(&sa_zombies.sa_mask);
  sa_zombies.sa_flags = SA_RESTART|SA_NOCLDSTOP|SA_NOCLDWAIT;
  int zombie = sigaction(SIGCHLD, &sa_zombies, NULL);

  if (zombie) {
    perror("sigaction");
    exit(2);
  }

  /* Wait for Child */

  background = command->background;
  if (!command->background) {
    waitpid(ret, NULL, 0);
  }

  free_command(command);

  if (isatty(0)) {
    print_prompt();
  }
} /* execute_command() */
