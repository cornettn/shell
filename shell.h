#ifndef SHELL_H
#define SHELL_H

#include "command.h"
#include "single_command.h"

void print_prompt();

extern command_t *g_current_command;
extern single_command_t *g_current_single_command;

int g_counter;
int g_max_entries;
char **g_array;

#endif
