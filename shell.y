
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

char *substring(char *str, int start, int end) {
  int size = end - start;
  char *sub = (char *) malloc(size * sizeof(char));

  for (int i = 0; i < size; i++) {
    *(sub + i) = *(str + start + i);
  }

  return sub;
}

/*
 * This function will take a string and remove quotes from the
 * start and the back.
 */

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

/*
 * This function will escape any characters that are preceded by a '\'
 */

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
  char *rest_of_string;
  int env_len = 0;
  for (int i = 0; i < len; i++) {
    if ((*(copy + i) == '{') && (*(copy + i - 1) == '$')) {

    /* There is an environement varaiable to escape.
     * The name of the env var start at index i + 1 */

    /* Get the length of the env var */

    for (int j = i + 1; j < len; j++) {
        if ((*(copy + j) == '}')) {
          rest_of_string = substring(copy, j + 1, len);
          break;
        }
        env_len++;
      }

      printf("Rest: %s\n", rest_of_string);
      env = (char *) malloc(env_len * sizeof(char));

      /* Get the actual value of the env var */

      for (int j = 0; j < env_len; j++) {
        *(env + j) = *(copy + i + 1 + j);
      }
      char *value = getenv(env);

      /* Replace the value of ${*} with the value */

      if (value != NULL) {
        int more_space = strlen(value) - env_len - 3;
        printf("Curr len: %d\n", len);
        printf("Len of val: %d\n", (int) strlen(value));
        if (more_space > 0) {
          printf("Realloc str to have %d more bytes\n", more_space);
          str = realloc(str, (len + more_space) * sizeof(char));
        }
        else {
          more_space = 0;
        }

        int val_len = strlen(value);
        int val_count = 0;
        int rest_count = 0;
        for (int j = i - 1; j < len + more_space; j++) {
          if (val_count < val_len) {
            /* Write the env var value */
            *(str + j) = *(value + val_count);
            val_count++;
          }
          else {
            /* Write the rest of the string */
            *(str + j) = *(rest_of_string + rest_count);
            rest_count++;
          }
        }
      }


      printf("String? \"%s\"\n", str);
      free(rest_of_string);
      free(env);
    } // if
  } // for

  free(copy);
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
