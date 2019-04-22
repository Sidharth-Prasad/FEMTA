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

hertz     { return HERTZ;     }
print     { return PRINT;     }
trigger   { return TRIGGER;   }
less_than { return LESS_THAN; }
more_than { return MORE_THAN; }

-?{digit}+ {
  yylval.number = atoi(yytext);
  return INT;
}

{id} {
  yylval.string = strdup(yytext);
  return ID;
}

[=\{\}] {
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
