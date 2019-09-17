
/*
 *
 * CS-252
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include "y.tab.h"

#include <stdio.h>

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return STDOUT;
}

">>" {
  return APPEND_STDOUT;
}

"<" {
  return INPUT;
}

[^ \t\n]+ {
  /* Assume that file names have only alpha chars */
  printf("%s\n", yytext);
  yylval.string = strdup(yytext);
  return WORD;
}
