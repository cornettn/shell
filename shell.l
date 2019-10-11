
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


extern char * read_line();

int mygetc(FILE *f) {
  static char *p;
  char ch;

  if (!isatty(0)) {

    /* Std In is not a terminal */

    return getc(f);
  }

  if (p == NULL || *p == 0) {
    char *s = read_line();
    p = s;
  }

  ch = *p;
  p++;

  return ch;

}

#undef getc
#define getc(f) mygetc(f)


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

$\(.*\) {
  printf("Test\n");
  char *str = strdup(yytext);
  char *count = str;
  count[strlen(str) - 1] = '\0';
  count += 2;

  yylval.string = strdup(count);
  free(str);

  return SUBSHELL;
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

