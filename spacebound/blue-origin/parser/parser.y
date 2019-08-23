
%{
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/color.h"
#include "../sensors/sensor.h"

extern FILE * yyin;

int yylex();
void yyerror(char * message);

typedef struct ParseNode ParseNode;

ParseNode * hertz_node(int hertz);
ParseNode * trigger_node(char * id, bool less, int threshold, int gpio);
ParseNode * print_node();

void build_sensor(char * id, List * statements);
%}

%code requires {
  
  #include <stdbool.h>
  #include "../structures/list.h"
  #include "../sensors/sensor.h"

  typedef enum ParseNodeType {
      HERTZ_NODE,
      TRIGGER_NODE,
      PRINT_NODE,
  } ParseNodeType;
  
  typedef struct ParseNode {

    int hertz;
    
    Trigger * trigger;

    bool print;

    ParseNodeType type;
    
  } ParseNode;
  
}

%union {
  char * string;
  int number;
  List * list;
  ParseNode * node;
}

%token <string> ID
%token <number> INT

%token PRINT HERTZ LESS_THAN MORE_THAN TRIGGER

%type <node> Statement
%type <list> Statements

%start Config

%%

Config     : Sensors
           ;

Sensors    : Sensor                
           | Sensors Sensor
           ;

Sensor     : ID '{'            '}'           { printf("%s has empty definition\n", $1); exit(1); }
           | ID '{' Statements '}'           { build_sensor($1, $3);                             }
           ;

Statements : Statement                       { $$ = list_from(1, $1);                            }
           | Statements Statement            { list_insert($1, $2); $$ = $1;                     }
           ;

Statement  : HERTZ '=' INT                   { $$ = hertz_node($3);                              }
           | ID LESS_THAN INT TRIGGER INT    { $$ = trigger_node($1,  true, $3, $5);             }
           | ID MORE_THAN INT TRIGGER INT    { $$ = trigger_node($1, false, $3, $5);             }
           | PRINT                           { $$ = print_node();                                }
           ;

%%

ParseNode * parse_node_create(ParseNodeType type) {
  
  ParseNode * node = malloc(sizeof(ParseNode));
  
  node -> type = type;
  
  node -> hertz     = 0;
  node -> trigger   = NULL;
  node -> print     = false;
  
  return node;
}

ParseNode * hertz_node(int hertz) {
  
  ParseNode * node = parse_node_create(HERTZ_NODE);
  
  node -> hertz = hertz;
  
  return node;
}

ParseNode * trigger_node(char * id, bool less, int threshold, int gpio) {
  
  ParseNode * node = parse_node_create(TRIGGER_NODE);
  
  node -> trigger = trigger_create(id, less, threshold, gpio);
  
  return node;
}

ParseNode * print_node() {
  return parse_node_create(PRINT_NODE);
}

void build_sensor(char * id, List * statements) {
  
  ProtoSensor * proto = hashmap_get(proto_sensors, id);
  
  if (!proto) {
    printf(RED "%s is not a sensor\n" RESET, id);
    exit(1);
  }
  
  for (iterate(statements, ParseNode *, node)) {
    
    switch (node -> type) {
    case HERTZ_NODE:
      proto -> hertz = node -> hertz;
      break;
      
    case TRIGGER_NODE:

      if (!proto -> triggers)
	proto -> triggers = list_create();
      
      Trigger * trigger = node -> trigger;
      
      if (!proto -> targets || !hashmap_exists(proto -> targets, trigger -> id)) {
	printf(RED "Trigger target %s unknown\n" RESET, trigger -> id);
	exit(1);
      }
      
      list_insert(proto -> triggers, trigger);
      
      break;
      
    case PRINT_NODE:
      proto -> print = true;
      break;
    }
  }
  
  proto -> requested = true;
}
