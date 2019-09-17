
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
  { printf("%s\n", $1); }
  ;

entire_command_list:
     entire_command_list entire_command
  |  entire_command
  ;

entire_command:
     single_command_list io_modifier_list NEWLINE
  |  NEWLINE
  ;

single_command_list:
     single_command_list PIPE single_command
  |  single_command
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
     WORD
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
