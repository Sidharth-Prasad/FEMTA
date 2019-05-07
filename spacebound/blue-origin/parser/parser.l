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
id     ({letter}|{digit})+

%%


\/\/.* {}                                                                    // 1 line comment
\/\*(([^*/]|\n)|(\*+([^*/]|\n)|\/([^*/]|\n)*|\/\*([^*/]|\n)*))*\**\*\/ {}    // multi comment

print     { return PRINT;     }
trigger   { return TRIGGER;   }
less_than { return LESS_THAN; }
more_than { return MORE_THAN; }

{digit}+Hz {
  yylval.integer = atoi(yytext);    // used by all sensors
  return HERTZ;
}

{digit}+s {
  yylval.integer = atoi(yytext);    // used by ds32
  return SECONDS;
}
{digit}+m {
  yylval.integer = atoi(yytext);    // used by ds32
  return MINUTES;
}

-?{digit}+c {
  yylval.integer = atoi(yytext);    // used by ad15
  return COUNTS;
}

-?{digit}+\.{digit}+g {
  yylval.decimal = atof(yytext);    // used by adxl
  return GS;
}

-?{digit}+\.{digit}+v {
  yylval.decimal = atof(yytext);
  return VOLTS;
}

(-|\+){digit}+ {
  yylval.integer = atoi(yytext);    // used by all sensors
  return CHARGE;
}

{id} {
  yylval.string = strdup(yytext);
  return ID;
}

[=\{\},;] {
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
