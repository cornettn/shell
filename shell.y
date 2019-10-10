
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
#include <assert.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

#define MAXFILENAME (1024)

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


/*
 * This function is used to get the value of the env variable. i.e. ${this}
 * char *env: The string of the environment variable to the value of.
 * return char *: The string corresponding to the value of env var.
 */

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
} /* get_value() */

/*
 * This function is used to escape any environment variables within the passed argument.
 * char *str: The argument to escape the variables from.
 * return char *: A pointer to the new argument with escaped variables.
 */

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
} /* escape_env_varaibles() */

/*
 * This function determines whether or not the string has any wildcards in it.
 * char *str: The string to test against.
 * return bool: True if wildcards are present, False otherwise.
 */

bool has_wildcards(char *str) {
  return (strchr(str, '?') != NULL) || (strchr(str, '*') != NULL);
} /* has_wildcards() */

/*
 * This function will convert the bash wildcarded string into a regex string.
 * char *str: The string to convert to regex.
 * return char *: A dynamically allocated pointer to a string containing
 *                regex version of str.
 */

char *to_regex(char *str) {
  char *regex = (char *) malloc(2 * strlen(str) + 3);
  char *arg_pos = str;
  char *regex_pos = regex;

  *regex_pos++ = '^';
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
} /* to_regex() */

/*
 * This function is used to get an array of matching strings.
 * char ** array: A pointer to the array that will be filled with matches.
 * DIR *dir: The directory to check through
 * regex_t reg: The regex structure to use for matching
 * char *regex: The regex string to check against
 * int *count: A pointer to the number of matches found
 * return char**: The array of all the matches.
 */

char **find_matching_strings(char **array, DIR *dir, regex_t reg,
                             char *str, int *count) {
    struct dirent *ent;
    int max_entries = 20;
    int num_entries = 0;
    array = (char **) malloc(max_entries * sizeof(char *));

    /* Iterate through the directory and find all matching strings */

    while ((ent = (struct dirent *)readdir(dir)) != NULL) {
      printf("Ent: %s\n", ent->d_name);
      int nmatch = 0;
      int status = regexec(&reg, ent->d_name, 0, NULL, 0);
      if (status != REG_NOMATCH) {
        printf("%s matches\n", ent->d_name);
        /* Match */
        bool add = false;
        if (ent->d_name[0] == '.') {
          if (str[0] == '.') {
            add = true;
          }
        }
        else {
          add = true;
        }

        if (add) {
          if (num_entries == max_entries) {
            max_entries *= 2;
            array = realloc(array, max_entries * sizeof(char *));
            assert(array != NULL);
          }
          array[num_entries] = strdup(ent->d_name);
          num_entries++;
        }
      }
    }
  *(count) = num_entries;
  return array;
} /* find_matching_strings() */

/*
 * This function is used in conjunction with qsort to sort strings.
 * return int: A negative number is a < b, 0 if a = b,
 *             positive number if a > b
 */

int my_compare(const void *a, const void *b) {
  char *str_a = *(char **) a;
  char *str_b = *(char **) b;
  return strcmp(str_a, str_b);
} /* my_compare() */


/*
 * This function will sort an array using qsort.
 * char **array: The array to sort.
 * int num: The number of elements to sort.
 */

void sort_array_strings(char **array, int num) {
  qsort(array, num, sizeof(char *), my_compare);
} /* sort_array() */


void add_item(char *item) {
  if (g_counter == g_max_entries) {
    g_max_entries *= 2;
    g_array = realloc(g_array, g_max_entries * sizeof(char *));
    assert(g_array != NULL);
  }

  g_array[g_counter] = strdup(item);
  g_counter++;
}


void expand_wildcards(char *prefix, char *suffix) {

  if (suffix[0] == '\0') {
    if (prefix[0] == '.') {
      if (g_curr_regex[0] == '.') {
        add_item(prefix);
      }
    }
    else {
      add_item(prefix);
    }
    //insert_argument(g_current_single_command, strdup(prefix));
    return;
  }

  /* Find location of next '/' */
  char *s = strchr(suffix, '/');
  char *component = (char *) malloc(MAXFILENAME);
  *component = '\0';
  if (s != NULL) {
    strncpy(component, suffix, s - suffix);

    /* Advance suffix */

    suffix = s + 1;
  }
  else {
    strcpy(component, suffix);
    suffix += strlen(suffix);
  }

  char *new_prefix = (char *) malloc(MAXFILENAME);
  *new_prefix = '\0';
  if (!has_wildcards(component)) {
    if (component[0] != '\0') {
      sprintf(new_prefix, "%s/%s", prefix, component);
    }
    free(component);
    expand_wildcards(new_prefix, suffix);
    free(new_prefix);
  }
  else {

    /* has wildcards */

    char *regex = to_regex(component);
    g_curr_regex = regex;
    regex_t reg;
    int status = regcomp(&reg, regex, REG_EXTENDED);
    if (status != 0) {
      perror("compile");
      return;
    }

    int test = 0;

    struct dirent *ent;
    char *directory = (prefix[0] == '\0') ? "." : prefix;
    DIR *dir = opendir(directory);
    if (dir == NULL) {
      return;
    }

    bool flag = false;
    while ((ent = readdir(dir)) != NULL) {

      if (ent->d_name[0] == '.') {
        if (component[0] != '.') {
          continue;
        }
      }

      int result = regexec(&reg, ent->d_name, 0, NULL, 0);
      if (result != REG_NOMATCH) {

        if (prefix[0] == '\0') {
          sprintf(new_prefix, "%s", ent->d_name);
        }
        else {
          sprintf(new_prefix, "%s/%s", prefix, ent->d_name);
        }
        expand_wildcards(new_prefix, suffix);
      }
      else {
        test++;
      }
    }

    free(component);
    free(new_prefix);
    closedir(dir);
  }
}

/*
 * This function is used to expand any wildcards that are in the passed
 * argument. i.e. '*', '?'
 * char *str: The argument to expand the wildcards from.
 */

void old_expand_wildcards(char *str) {
  if (!has_wildcards(str)) {

    /* No wildcards */

    insert_argument(g_current_single_command, str);
    return;
  }
  else {

    /* Wild cards are present */

    char *regex = to_regex(str);
    printf("regex:%s\n", regex);
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

    char **array = (char **) malloc(sizeof(char *));
    int *count = (int *) malloc(sizeof(int));
    array = find_matching_strings(array, dir, reg, str, count);

    closedir(dir);

    sort_array_strings(array, *count);

    for (int i = 0; i < *count; i++) {
      insert_argument(g_current_single_command, strdup(array[i]));
      free(array[i]);
    }

    free(array);

  } // else
} /* expand_wildcards() */

/*
 * This function is used to expand the argument to its full meeaning.
 * char *str: The argument to expand.
 */

void expand_argument(char * str) {
  char *passed_str = str;
  bool quoted = false;
  g_max_entries = 20;
  g_counter = 0;
  g_array = (char **) malloc(g_max_entries * sizeof(char *));

  /* Returns the char pointer without quotes in it*/

  char *argument = quoted_arg(str);
  if (strcmp(argument, passed_str) != 0) {
    quoted = true;
  }

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

  if (!quoted) {
    char *prefix = (char *) malloc(MAXFILENAME);
    *prefix = '\0';
    if (has_wildcards(argument)) {
      if (strchr(argument, '/') == NULL) {
        old_expand_wildcards(argument);
      }
      else {
        expand_wildcards(prefix, argument);

        sort_array_strings(g_array, g_counter);

        for (int i = 0; i < g_counter; i++) {
          insert_argument(g_current_single_command, strdup(g_array[i]));
        }


        for (int i = 0; i < g_counter; i++) {
          free(g_array[i]);
        }
        free(g_array);
      }
    }
    else {
      insert_argument(g_current_single_command, argument);
    }
    free(prefix);
  }
  else {
    insert_argument(g_current_single_command, argument);
  }
} /* exapnd_argument() */


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
