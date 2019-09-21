
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

"&" {
  return BACKGROUND;
}

"|" {
  return PIPE;
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

">&" {
  return STDERR;
}

">>&" {
  return APPEND_STDERR;
}

[^ \t\n]+ {
  /* Assume that file names have only alpha chars */
  yylval.string = strdup(yytext);
  return WORD;
}
