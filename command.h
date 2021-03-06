#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

#include <sys/wait.h>
#include "single_command.h"

// Command Data Structure

typedef struct command {
  char *out_file;
  char *in_file;
  char *err_file;
  bool append_out;
  bool append_err;
  bool background;

  single_command_t **single_commands;
  int num_single_commands;
} command_t;

typedef struct node {
  struct node *next;
  int pid;
} node_t;

void create_command(command_t *);
void insert_single_command(command_t *, single_command_t *);
void free_command(command_t *);
void print_command(command_t *);
void execute_command(command_t *);

extern command_t *g_current_command;
command_t *g_last_command;

int g_last_background_PID;
char *g_last_argument;
char *g_shell_name;
int g_status;

#endif
