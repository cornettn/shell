
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires
{

}

%union
{
  char * string;
}

%token <string> WORD PIPE
%token NOTOKEN NEWLINE STDOUT
%token INPUT BACKGROUND APPEND_STDOUT
%token STDERR APPEND_STDOUT_STDERR
%token STDOUT_STDERR

%{

#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

char *quoted_arg(char *str) {
  int str_len = strlen(str);
  if ((*str) == '\"') {
    str++;
    char *passed_str = (char *)malloc(str_len * (sizeof(char)));
    for (int i = 0; i < str_len; i++) {
      if ((*str) != '\"') {
        *(passed_str + i) = *str;
      }
      else {
        *(passed_str + i) = '\0';
        break;
      }
      str++;
    }
    return passed_str;
  }
  else {
    return str;
  }
}

char *escape_characters(char *str) {
  int str_len = strlen(str);
  for (int i = 0; i < str_len; i++) {
    if (*(str + i) == '\\') {
      *(str + i) = *(str + i + 1);
      for (int j = i + 1; j <= str_len; j++) {
        *(str + j) = *(str + j + 1);
      }
      *(str + str_len) = '\0';
      str_len = strlen(str);
    }
  }
  return str;
}


char *escape_env_variables(char *str) {
  int len = strlen(str);
  char *copy = strdup(str);
  char *env;
  int env_len = 0;
  for (int i = 0; i < len; i++) {
    if ((*(copy + i) == '{') && (*(copy + i - 1) == '$')) {
      for (int j = i + 1; j < len; j++) {
        if ((*(copy + j) == '}')) {
          break;
        }
        env_len++;
      }
      env = (char *) malloc(env_len * sizeof(char));
      for (int j = 0; j < env_len; j++) {
        *(env + j) = *(copy + i + 1 + j);
      }
      printf("Var: %s\n", env);
      char *value = getenv(env);
      if (value != NULL) {
        printf("Val: %s\n", value);
        int more_space = len + 3 - strlen(value);
        if (more_space > 0) {
          str = realloc(str, (len + more_space) * sizeof(char));
        }
      }
    } // if
  } // for
  return str;
}


void expand_argument(char * str) {
  char *passed_str = str;

  /* Returns the char pointer without quotes in it*/

  char *argument = quoted_arg(str);

  /* Returns the char pointer where characters following
   * a '/' have been escaped */

  argument = escape_characters(argument);

  argument = escape_env_variables(argument);



  insert_argument(g_current_single_command, argument);
}


%}

%%

goal:
  entire_command_list
  ;

entire_command_list:
    entire_command_list entire_command {
      execute_command(g_current_command);
      g_current_command = malloc(sizeof(command_t));
      create_command(g_current_command);
    }
  | entire_command {
      execute_command(g_current_command);
      g_current_command = malloc(sizeof(command_t));
      create_command(g_current_command);
    }
  ;

entire_command:
    single_command_list io_modifier_list NEWLINE
  | single_command_list io_modifier_list BACKGROUND NEWLINE {
    g_current_command->background = true;
  }
  |  NEWLINE
  ;

single_command_list:
    single_command_list PIPE single_command {
    }
  | single_command {
    }
  ;

single_command:
    executable argument_list {
      insert_single_command(g_current_command, g_current_single_command);
    }
  ;

argument_list:
    argument_list argument
  |  /* can be empty */
  ;

argument:
     WORD {
      expand_argument($1);
/*      insert_argument(g_current_single_command, $1); */
    }
  ;

executable:
     WORD {
      g_current_single_command = malloc(sizeof(single_command_t));
      create_single_command(g_current_single_command);

      expand_argument($1);
/*      insert_argument(g_current_single_command, $1); */
    }
  ;

io_modifier_list:
     io_modifier_list io_modifier
  |  io_modifier
  |  /* can be empty */
  ;

io_modifier:
     STDOUT WORD {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->out_file = $2;
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
  |  INPUT WORD {
      g_current_command->in_file = $2;
    }
  | APPEND_STDOUT WORD {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->append_out = true;
      g_current_command->out_file = $2;
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_APPEND|O_RDWR, 0600);
      close(fd);
    }
  | STDERR WORD {
      g_current_command->err_file = $2;
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
  | STDOUT_STDERR WORD {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->out_file = $2;
      g_current_command->err_file = strdup($2);
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
  | APPEND_STDOUT_STDERR WORD {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->append_out = true;
      g_current_command->append_err = true;
      g_current_command->err_file = $2;
      g_current_command->out_file = strdup($2);
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_APPEND|O_RDWR, 0600);
      close(fd);
    }
  ;


%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
