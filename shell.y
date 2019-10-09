
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
%token ENV

%{

#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

/*
 * This function returns a newly allocated char pointer that is the substring.
 */

char *substring(char *str, int start, int end) {
  int size = end - start;
  char *sub = (char *) malloc(size * sizeof(char));

  for (int i = 0; i < size; i++) {
    *(sub + i) = *(str + start + i);
  }
  *(sub + size) = '\0';

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

/*
 * This fucntion is used to replace environement variables with their
 * respective value.
 * char * str: The str to insert the value into
 * int i: The index of the '{' in ${VARIABLE} in the str
 * int env_len: The length of the environment varaible
 * char * value: The value of the variable to insert
 * char * remaining_str: The remainder of the string after the env variable.
 */

char *replace_env(char *str, int i, int env_len, char *value,
                  char *remaining_str) {
  int len = strlen(str);
  if (value != NULL) {
    int more_space = strlen(value) - env_len - 3;
    str = realloc(str, (len + more_space) * sizeof(char));

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
        *(str + j) = *(remaining_str + rest_count);
        rest_count++;
      }
    }
    *(str + len + more_space) = '\0';
  }
  return str;
}


char *get_value(char *env) {
  char *str;
  if (!strcmp(env, "$")) {
    str = (char *) malloc(10 * sizeof(char));
    sprintf(str, "%ld", (long) getpid());
    return str;
  }
  else if (!strcmp(env, "?")) {
    str = (char *) malloc(sizeof(char));
    sprintf(str, "%d", g_status);
    return str;
  }
  else if (!strcmp(env, "!")) {
    str = (char *) malloc(10 * sizeof(char));
    sprintf(str, "%ld", (long) g_last_background_PID);
  }
  else if (!strcmp(env, "!!")) {
    execute_command(g_last_command);
    return NULL;
  }
  else if (!strcmp(env, "_")) {
    return strdup(g_last_argument);
  }
  else if (!strcmp(env, "SHELL")) {
    char *path = realpath(g_shell_name, path);
    return path;
  }
  else {
    str = strdup(getenv(env));
  }
  return str;
}


char *escape_env_variables(char *str) {
  int len = strlen(str);
  char *env;
  char *rest_of_string;
  for (int i = 0; i < len; i++) {
    int env_len = 0;
    if ((*(str + i) == '{') && (*(str + i - 1) == '$')) {
    /* There is an environement varaiable to escape.
     * The name of the env var start at index i + 1 */

    /* Get the length of the env var */

    for (int j = i + 1; j < len; j++) {
        if ((*(str + j) == '}')) {
          rest_of_string = substring(str, j + 1, len);
          break;
        }
        env_len++;
      }

      env = (char *) malloc((env_len + 1) * sizeof(char));

      /* Get the actual value of the env var */

      for (int j = 0; j < env_len; j++) {
        *(env + j) = *(str + i + 1 + j);
      }
      *(env + env_len) = '\0';

      char *value = get_value(env);
      if (value == NULL) {
        free(value);
        free(rest_of_string);
        free(env);
        return NULL;
      }

      /* Replace the value of ${*} with the value */

      str = replace_env(str, i, env_len, value, rest_of_string);

      len = strlen(str);
      free(value);
      free(rest_of_string);
      free(env);
    } // if
  } // for

  return str;
}


bool has_wildcards(char *str) {
  return ((strchr(str, '?') != NULL || (strchr(str, '.') != NULL;))
}


char *to_regex(char *str) {
  char *regex = (char *) malloc(2 * strlen(str) + 3);
  char *arg_pos = str;
  char *regex_pos = regex;

  *reg_pos++ = '^';
  while(*arg_pos) {
    if (*arg_pos == '*') {
      *regex_pos++ = '.';
      *regex_pos++ = '*';
    }
    else if (*arg_pos == '?') {
      *regex_pos++ = '.';
    }
    else if (*arg_pos == '.') {
      *regex_pos++ = '\\';
      *regex_pos == '.';
    }
    else {
      *regex_pos++ = *arg_pos;
    }

    arg_pos++;
  }
  *regex_pos++ = '$';
  *regex_pos = '\0';
  return regex;
}


void expand_wildcards(char *str) {
  if (!has_wildcards(str)) {
    /* No wildcards */
    insert_argument(g_current_single_command, str);
    return;
  }
  else {
    /* Wild cards are present */
    char *regex = to_regex(str);
    regex_t reg;
    int status = regcomp(&reg, regex, REG_EXTENDED);
    if (status != 0) {
      perror("compile");
      return;
    }

    DIR *dir = opendir(".");
    if (dir == NULL) {
      perror("opendir");
      return;
    }

    struct direct *ent;

    while ((ent = readdir(dir)) != NULL) {
      int nmatch = 0;
      regmatch_t match[nmatch];
      status = regexec(&reg, regex, nmatch, match, 0);
      if (status != REG_NOMATCH) {
        insert_argument(g_current_single_command, strdup(ent->d_name));
      }
    }
    closedir(dir);
  } // else
}

void expand_argument(char * str) {
  char *passed_str = str;

  /* Returns the char pointer without quotes in it*/

  char *argument = quoted_arg(str);

  /* Returns the char pointer where characters following
   * a '/' have been escaped */

  argument = escape_characters(argument);

  argument = escape_env_variables(argument);
  if (argument == NULL) {
    free_command(g_current_command);
    free_single_command(g_current_single_command);
    yyparse();
    return;
  }

  expand_wildcards(argument);

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
