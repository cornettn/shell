/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */

#line 67 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 15 "shell.y" /* yacc.c:355  */



#line 101 "y.tab.c" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    WORD = 258,
    PIPE = 259,
    NOTOKEN = 260,
    NEWLINE = 261,
    STDOUT = 262,
    INPUT = 263,
    BACKGROUND = 264,
    APPEND_STDOUT = 265,
    STDERR = 266,
    APPEND_STDOUT_STDERR = 267,
    STDOUT_STDERR = 268,
    SUBSHELL = 269
  };
#endif
/* Tokens.  */
#define WORD 258
#define PIPE 259
#define NOTOKEN 260
#define NEWLINE 261
#define STDOUT 262
#define INPUT 263
#define BACKGROUND 264
#define APPEND_STDOUT 265
#define STDERR 266
#define APPEND_STDOUT_STDERR 267
#define STDOUT_STDERR 268
#define SUBSHELL 269

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 20 "shell.y" /* yacc.c:355  */

  char * string;

#line 145 "y.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 31 "shell.y" /* yacc.c:358  */


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


char *replace_tilde(char *str, int tilde_index, char *tilde_value) {
  int len = strlen(str);
  int more_space = strlen(tilde_value) - 1;
  str = realloc(str, (strlen(str) + more_space) * sizeof(char));
  char * rest_of_string = substring(str, tilde_index, strlen(str));
  int rest_count = 0;

  int tilde_value_len = strlen(tilde_value);
  int tilde_count = 0;
  for (int i = tilde_index; i < len + more_space; i++) {
    if (tilde_count < tilde_value_len) {

      /* Writing val of tilde to string */

      *(str + i) = *(tilde_value + tilde_count++);
    }
    else {

      /* Write the raiming string to str */

      *(str + i) = *(rest_of_string + rest_count++);
    }
  }
  *(str + len + more_space) = 0;
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
  //printf("Malloc regex\n");
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
      *regex_pos++ = '.';
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

    //printf("Malloc array\n");
    array = (char **) malloc(max_entries * sizeof(char *));

    /* Iterate through the directory and find all matching strings */

    while ((ent = (struct dirent *)readdir(dir)) != NULL) {
      //printf("Ent: %s\n", ent->d_name);
      int nmatch = 0;
      int status = regexec(&reg, ent->d_name, 0, NULL, 0);
      if (status != REG_NOMATCH) {
        //printf("%s matches\n", ent->d_name);
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
            //printf("realloc array\n");
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
    //printf("realloc g_array\n");
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
  //printf("Malloc component\n");
  char *component = (char *) malloc(MAXFILENAME);
  *component = '\0';
  if (s != NULL) {
    strncpy(component, suffix, s - suffix);
    component[s - suffix] = '\0';
    /* Advance suffix */

    suffix = s + 1;
  }
  else {
    strcpy(component, suffix);
    suffix += strlen(suffix);
  }

  //printf("Malloc new_prefix\n");
  char *new_prefix = (char *) malloc(MAXFILENAME);
  *new_prefix = '\0';
  if (!has_wildcards(component)) {
    if (component[0] != '\0') {
      sprintf(new_prefix, "%s/%s", prefix, component);
    }
    //printf("Free compo\n");
    free(component);
    expand_wildcards(new_prefix, suffix);
    //printf("Free new_prefix\n");
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

    free(regex);
    regex = NULL;

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

    //printf("Free component\n");

    regfree(&reg);
    free(component);
    component = NULL;
    //printf("Free new_prefix\n");
    free(new_prefix);
    new_prefix = NULL;
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
    //printf("regex:%s\n", regex);
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

    //printf("malloc array\n");
    char **array = NULL;
    //printf("malloc count\n");
    int *count = (int *) malloc(sizeof(int));
    array = find_matching_strings(array, dir, reg, str, count);

    closedir(dir);

    sort_array_strings(array, *count);

    for (int i = 0; i < *count; i++) {
      insert_argument(g_current_single_command, strdup(array[i]));
      //printf("free array[i]\n");
      free(array[i]);
    }
    //printf("free array\n");


    regfree(&reg);
    free(array);
    array = NULL;
    free(count);
    count = NULL;
    free(regex);
    regex = NULL;
  } // else
} /* expand_wildcards() */


char *expand_tilde(char *str) {
  char *tilde = strchr(str, '~');
  char *slash = strchr(str, '/');

  if (tilde == NULL) {

  /* Tilde is not present in the passed string */

  return str;
  }

  char *username = NULL;

  if (slash != NULL) {
    username = substring(str, tilde - str + 1, slash - str);
  }
  else {
    username = substring(str, tilde - str + 1, strlen(str));
  }
  char *value = getenv("HOME");
  char *directory_slash = strchr(value + 1, '/');
  char *directory = substring(value, 0, directory_slash - value);

  str = replace_tilde(str, tilde - str, value);
  return str;
}


/*
 * This function is used to expand the argument to its full meeaning.
 * char *str: The argument to expand.
 */

void expand_argument(char * str) {
  char *passed_str = str;
  bool quoted = false;

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
    argument = expand_tilde(argument);
  }

  if (!quoted) {
    g_max_entries = 20;
    g_counter = 0;
    char *prefix = (char *) malloc(MAXFILENAME);
    *prefix = '\0';
    if (has_wildcards(argument)) {
      if (strchr(argument, '/') == NULL) {
        old_expand_wildcards(argument);
      }
      else {
        g_array = (char **) malloc(g_max_entries * sizeof(char *));
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



#line 815 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   36

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  15
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  11
/* YYNRULES -- Number of rules.  */
#define YYNRULES  23
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  34

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   688,   688,   692,   697,   705,   706,   709,   713,   715,
     720,   726,   727,   731,   738,   748,   749,   750,   754,   764,
     767,   778,   784,   795
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "WORD", "PIPE", "NOTOKEN", "NEWLINE",
  "STDOUT", "INPUT", "BACKGROUND", "APPEND_STDOUT", "STDERR",
  "APPEND_STDOUT_STDERR", "STDOUT_STDERR", "SUBSHELL", "$accept", "goal",
  "entire_command_list", "entire_command", "single_command_list",
  "single_command", "argument_list", "argument", "executable",
  "io_modifier_list", "io_modifier", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269
};
# endif

#define YYPACT_NINF -5

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-5)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -1,    -5,    -5,     1,    -1,    -5,    -4,    -5,    -5,    -5,
      -5,    15,    16,    17,    18,    19,    20,    21,     4,    -5,
      22,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    23,
      -5,    -5,    -5,    -5
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,     7,     0,     2,     4,    17,     9,    12,     1,
       3,     0,     0,     0,     0,     0,     0,     0,     0,    16,
      10,     8,    18,    19,    20,    21,    23,    22,     5,     0,
      15,    13,    11,     6
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -5,    -5,    -5,    24,    -5,    25,    -5,    -5,    -5,    -5,
       8
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,     7,    20,    32,     8,    18,
      19
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      11,     9,     1,    12,    13,     2,    14,    15,    16,    17,
      28,    12,    13,    29,    14,    15,    16,    17,     1,    22,
      23,    24,    25,    26,    27,    31,    30,     0,    10,    33,
       0,     0,     0,     0,     0,     0,    21
};

static const yytype_int8 yycheck[] =
{
       4,     0,     3,     7,     8,     6,    10,    11,    12,    13,
       6,     7,     8,     9,    10,    11,    12,    13,     3,     3,
       3,     3,     3,     3,     3,     3,    18,    -1,     4,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    11
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     6,    16,    17,    18,    19,    20,    23,     0,
      18,     4,     7,     8,    10,    11,    12,    13,    24,    25,
      21,    20,     3,     3,     3,     3,     3,     3,     6,     9,
      25,     3,    22,     6
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    15,    16,    17,    17,    18,    18,    18,    19,    19,
      20,    21,    21,    22,    23,    24,    24,    24,    25,    25,
      25,    25,    25,    25
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     3,     4,     1,     3,     1,
       2,     2,     0,     1,     1,     2,     1,     0,     2,     2,
       2,     2,     2,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 692 "shell.y" /* yacc.c:1646  */
    {
      execute_command(g_current_command);
      g_current_command = malloc(sizeof(command_t));
      create_command(g_current_command);
    }
#line 1913 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 697 "shell.y" /* yacc.c:1646  */
    {
      execute_command(g_current_command);
      g_current_command = malloc(sizeof(command_t));
      create_command(g_current_command);
    }
#line 1923 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 706 "shell.y" /* yacc.c:1646  */
    {
    g_current_command->background = true;
  }
#line 1931 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 713 "shell.y" /* yacc.c:1646  */
    {
    }
#line 1938 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 715 "shell.y" /* yacc.c:1646  */
    {
    }
#line 1945 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 720 "shell.y" /* yacc.c:1646  */
    {
      insert_single_command(g_current_command, g_current_single_command);
    }
#line 1953 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 731 "shell.y" /* yacc.c:1646  */
    {
      expand_argument((yyvsp[0].string));
/*      insert_argument(g_current_single_command, $1); */
    }
#line 1962 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 738 "shell.y" /* yacc.c:1646  */
    {
      g_current_single_command = malloc(sizeof(single_command_t));
      create_single_command(g_current_single_command);

      expand_argument((yyvsp[0].string));
/*      insert_argument(g_current_single_command, $1); */
    }
#line 1974 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 754 "shell.y" /* yacc.c:1646  */
    {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->out_file = (yyvsp[0].string);
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
#line 1989 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 764 "shell.y" /* yacc.c:1646  */
    {
      g_current_command->in_file = (yyvsp[0].string);
    }
#line 1997 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 767 "shell.y" /* yacc.c:1646  */
    {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->append_out = true;
      g_current_command->out_file = (yyvsp[0].string);
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_APPEND|O_RDWR, 0600);
      close(fd);
    }
#line 2013 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 778 "shell.y" /* yacc.c:1646  */
    {
      g_current_command->err_file = (yyvsp[0].string);
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
#line 2024 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 784 "shell.y" /* yacc.c:1646  */
    {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->out_file = (yyvsp[0].string);
      g_current_command->err_file = strdup((yyvsp[0].string));
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_TRUNC|O_RDWR, 0600);
      close(fd);
    }
#line 2040 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 795 "shell.y" /* yacc.c:1646  */
    {
      if (g_current_command->out_file) {
        printf("Ambiguous output redirect.\n");
      }

      g_current_command->append_out = true;
      g_current_command->append_err = true;
      g_current_command->err_file = (yyvsp[0].string);
      g_current_command->out_file = strdup((yyvsp[0].string));
      int fd = open(g_current_command->out_file,
                    O_CREAT|O_APPEND|O_RDWR, 0600);
      close(fd);
    }
#line 2058 "y.tab.c" /* yacc.c:1646  */
    break;


#line 2062 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 811 "shell.y" /* yacc.c:1906  */


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
