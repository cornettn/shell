
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

static void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

"\n" {
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

"2>" {
  return STDERR;
}

">&" {
  return STDOUT_STDERR;
}

">>&" {
  return APPEND_STDOUT_STDERR;
}

(\"[^\n\t]+\") {
  yylval.string = strdup(yytext);
  return WORD;
}

(([^ \t\n\>\<\&\|])|(\\.))+ {
  /* Assume that file names have only alpha characters */
  yylval.string = strdup(yytext);
  return WORD;
}

