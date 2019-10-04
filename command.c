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
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "shell.h"

//typedef struct node {
//  node_t *next;
//  size_t pid;
//} node_t;

node_t *g_background_process_head = NULL;
int g_debug;

/*
 *
 */

void append_background_process(int pid) {
  g_last_background_PID = pid;
  if (g_background_process_head == NULL) {
    g_background_process_head = (node_t *) malloc(sizeof(node_t *));
    g_background_process_head->next = NULL;
    g_background_process_head->pid = pid;
  }
  else {
    node_t *current = g_background_process_head;
    while (current->next != NULL) {
      current = current->next;
    }

    current->next = (node_t *) malloc(sizeof(node_t));
    current->next->next = NULL;
    current->next->pid = pid;
  }
}

/*
 *
 */

int is_background_process(int sid) {
  node_t *current = g_background_process_head;
  if (current == NULL) {
    return 0;
  }

  while (current != NULL) {
    if (current->pid == sid) {
      return 1;
    }
    current = current->next;
  }

  return 0;
}

/*
 * This function is responsible for handling zombie processes.
 */

void sig_child_handler(int sid) { //, siginfo_t *info, void *ucontext) {
  if (is_background_process(sid)) {
    printf("[%d] exited.\n", sid);
    print_prompt();
  }
}

/*
 * This function is responsible for changing the directory.
 */

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
    free_command(g_last_command);
    free(g_last_argument);
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
  else if (!strcmp(single->arguments[0], "source")) {
    printf("source command not implemented\n");
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
  if (command == NULL) {
    return;
  }
  else if (command->single_commands == NULL) {
    return;
  }

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

command_t *command_dup(command_t *command) {
  command_t *new_command = (command_t *) malloc(sizeof(command_t));
  create_command(new_command);
  if (command->out_file) {
    new_command->out_file = strdup(command->out_file);
  }
  if (command->in_file) {
    new_command->in_file = strdup(command->in_file);
  }
  if (command->err_file) {
    new_command->err_file = strdup(command->err_file);
  }

  new_command->append_out = command->append_out;
  new_command->append_err = command->append_err;
  new_command->background = command->background;

  for (int i = 0; i < command->num_single_commands; i++) {
    single_command_t *single = (single_command_t *) malloc(sizeof(single_command_t));
    create_single_command(single);
    for (int j = 0; j < command->single_commands[i]->num_args; j++) {
      char *arg = strdup(command->single_commands[i]->arguments[j]);
      insert_argument(single, arg);
    }
    insert_single_command(new_command, single);
  }
  return new_command;
}

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

  if (command->num_single_commands == 0) {
    if (isatty(0)) {
      print_prompt();
    }
    return;
  }

  /* Don't do anything if there are no single commands */

  if (command->single_commands == NULL) {
    int input = dup(0);
    if (isatty(input)) {
      print_prompt();
    }
    close(input);
    return;
  }

  /* Copy this command to g_last_command */

  if (g_last_command == NULL) {
    g_last_command = command_dup(command);
  }

  if (g_last_command != NULL && command != g_last_command) {
    free_command(g_last_command);
    g_last_command = command_dup(command);
  }

  single_command_t *last_single =
    g_last_command->single_commands[g_last_command->num_single_commands - 1];
  free(g_last_argument);
  g_last_argument =
    strdup(last_single->arguments[last_single->num_args - 1]);

  g_debug = open("debug", O_CREAT|O_RDWR|O_APPEND, 0600);



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
    dprintf(g_debug, "Is Bultin: %d\n", builtin);

    if (!builtin) {

      /* Fork if the command is not a builtin */

      dprintf(g_debug, "Fork\n");

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
    } // end if builtin
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

  if (!command->background) {
    waitpid(ret, g_last_exit_code, 0);
    if (WIFEXITED(*g_last_exit_code)) {
      *g_last_exit_code = WEXITSTATUS(*g_last_exit_code);
    }
  }
  else {
    append_background_process(ret);
  }

  if (command != g_last_command) {
    free_command(command);
  }

  if (isatty(0) && command != g_last_command) {
    print_prompt();
  }
} /* execute_command() */
