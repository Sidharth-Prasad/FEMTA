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
#line 39 "parser.y" /* yacc.c:1909  */

  
  #include <stdbool.h>
  #include "../math/units.h"
  #include "../structures/list.h"
  #include "../system/gpio.h"
  #include "../sensors/sensor.h"
  
  typedef struct EffectNode {
    
    bool  is_charge;    // whether effect is a charge or transition
    float delay_ms;
    
    union {
      Charge * charge;
      
      struct {
	char * state_name;
	bool   entering;
      };
    };
    
  } EffectNode;
  
  typedef struct Specification {
      
      char * id;
      List * options;
      List * args;
    
  } Specification;

#line 77 "y.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IF = 258,
    SET = 259,
    ENTER = 260,
    LEAVE = 261,
    AFTER = 262,
    STATE = 263,
    PIN = 264,
    POS = 265,
    NEG = 266,
    DEFINE = 267,
    ID = 268,
    NUMERIC = 269
  };
#endif
/* Tokens.  */
#define IF 258
#define SET 259
#define ENTER 260
#define LEAVE 261
#define AFTER 262
#define STATE 263
#define PIN 264
#define POS 265
#define NEG 266
#define DEFINE 267
#define ID 268
#define NUMERIC 269

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 72 "parser.y" /* yacc.c:1909  */

  char          * string;
  int           * integer;
  float         * decimal;
  Numeric       * numeric;
  
  List          * list;
  EffectNode    * effect;
  Trigger       * trigger;
  Specification * specification;

#line 129 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
