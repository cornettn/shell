
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


%{

#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

#include "command.h"
#include "single_command.h"
#include "shell.h"

void yyerror(const char * s);
int yylex();

%}

%%

goal:
  entire_command_list
  {
    printf("goal\n");
  }
  ;

entire_command_list:
    entire_command_list entire_command {
      printf("entire\n");
    }
  | entire_command {
      printf("entire\n");
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
      printf("single_command_list\n");
    }
  | single_command {
      printf("single_command_list\n");
      /* create_single_command($1); */
    }
  ;

single_command:
    executable argument_list
  ;

argument_list:
    argument_list argument
  |  /* can be empty */
  ;

argument:
     WORD
  ;

executable:
     WORD {
      g_current_single_command = malloc(sizeof(single_command));
      create_single_command(g_current_single_command);
      printf("rough\n");
    }
  ;

io_modifier_list:
     io_modifier_list io_modifier
  |  io_modifier
  |  /* can be empty */
  ;

io_modifier:
     STDOUT WORD
  |  INPUT WORD
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
