%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "y.tab.h"

void yyerror(char * message);
%}

%option yylineno

white  [ \t\r\n]+
letter [A-Za-z_]
digit  [0-9]

%%

\/\/.* {}                                                                    // 1 line comment
\/\*(([^*/]|\n)|(\*+([^*/]|\n)|\/([^*/]|\n)*|\/\*([^*/]|\n)*))*\**\*\/ {}    // multi comment

trigger   { return TRIGGER;   }
less_than { return LESS_THAN; }
more_than { return MORE_THAN; }

(-|\+)?{digit}+{letter}+ {
  // note, units may never exceed 7 characters
  
  yylval.numeric = malloc(sizeof(*yylval.numeric));
  
  sscanf(yytext, "%d%s", &yylval.numeric -> integer, yylval.numeric -> units);
  
  return NUMERIC;
}

(-|\+)?{digit}+\.{digit}+{letter}+ {
  // note, units may never exceed 7 characters
  
  yylval.numeric = malloc(sizeof(*yylval.numeric));
  
  sscanf(yytext, "%f%s", &yylval.numeric -> decimal, yylval.numeric -> units);
  return NUMERIC;
}

(-|\+)?{digit}+ {
  yylval.numeric = calloc(1, sizeof(*yylval.numeric));
  
  yylval.numeric -> integer  = atoi(yytext);
  yylval.numeric -> units[0] = 'i';             // the generic 32-bit int
  
  return NUMERIC;
}

(-|\+)?{digit}+\.{digit}+ {
  yylval.numeric = calloc(1, sizeof(*yylval.numeric));
  
  yylval.numeric -> decimal  = atof(yytext);
  yylval.numeric -> units[0] = 'f';             // the generic 32-bit float
  
  return NUMERIC;
}

({letter}|{digit})+ {
  yylval.string = strdup(yytext);
  return ID;
}

[=\{\}\+\-,:;\[\]] {
  return yytext[0];
}

{white} {}

. {
  printf("%s\n", yytext);
  yyerror("Unexpected symbol found");
}

%%

void yyerror(char * message) {
  printf("Line %d : %s\n", yylineno, message);
  exit(1);
}
