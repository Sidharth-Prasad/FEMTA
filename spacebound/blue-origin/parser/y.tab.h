/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 30 "parser.y" /* yacc.c:1909  */

  
  #include <stdbool.h>
  #include "../structures/list.h"
  #include "../sensors/sensor.h"

  typedef struct Threshold {
    bool use_decimal;
    union {
      int integer;
      double decimal;
    };
  } Threshold;

#line 59 "y.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PRINT = 258,
    LESS_THAN = 259,
    MORE_THAN = 260,
    TRIGGER = 261,
    ID = 262,
    HERTZ = 263,
    COUNTS = 264,
    CHARGE = 265,
    SECONDS = 266,
    VOLTS = 267,
    GS = 268
  };
#endif
/* Tokens.  */
#define PRINT 258
#define LESS_THAN 259
#define MORE_THAN 260
#define TRIGGER 261
#define ID 262
#define HERTZ 263
#define COUNTS 264
#define CHARGE 265
#define SECONDS 266
#define VOLTS 267
#define GS 268

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 45 "parser.y" /* yacc.c:1909  */

  char * string;
  int    integer;
  double decimal;
  List      * list;
  Threshold * threshold;
  Trigger   * trigger;

#line 106 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
