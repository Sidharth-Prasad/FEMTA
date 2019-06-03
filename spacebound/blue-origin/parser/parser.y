
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

typedef struct Charge Charge;
typedef struct Threshold Threshold;
typedef struct Trigger Trigger;

Charge    * make_charge(int raw_charge);
Threshold * make_threshold(bool use_decimal, int integer, double decimal);
Trigger   * make_trigger(List * charges, List * options);
Trigger   * modify_trigger(char * id, bool less, Threshold * threshold, Trigger * trigger);
 
void build_sensor(char * id, int hertz, List * triggers, bool print);
void print_config();
%}

%code requires {
  
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
}

%union {
  char * string;
  int    integer;
  double decimal;
  List      * list;
  Threshold * threshold;
  Trigger   * trigger;
}

%token PRINT LESS_THAN MORE_THAN TRIGGER

%token <string> ID
%token <integer> HERTZ COUNTS CHARGE SECONDS VOLTS    // should change volts to decimal later
%token <decimal> GS

%type <list>      Actions Charges Options
%type <threshold> Argument
%type <trigger> Actuator Action

%start Config

%%

Config   : Sensors
         ;

Sensors  : Sensor                
         | Sensors Sensor
         ;

Sensor   : ID HERTZ '{'         '}'       ';'         { build_sensor($1, $2, NULL, false);                }
         | ID HERTZ '{'         '}' PRINT ';'         { build_sensor($1, $2, NULL,  true);                }
         | ID HERTZ '{' Actions '}'       ';'         { build_sensor($1, $2,   $4, false);                }
         | ID HERTZ '{' Actions '}' PRINT ';'         { build_sensor($1, $2,   $4,  true);                }
         ;

Actions  : Action                                     { $$ = list_from(1, $1);                            }
         | Actions Action                             { list_insert($1, $2); $$ = $1;                     }
         ;

Action   : ID LESS_THAN Argument TRIGGER Actuator     { $$ = modify_trigger($1,  true, $3, $5);           }
         | ID MORE_THAN Argument TRIGGER Actuator     { $$ = modify_trigger($1, false, $3, $5);           }
         ;

Argument : VOLTS                                      { $$ = make_threshold(false, $1,   0);              }
         | GS                                         { $$ = make_threshold( true,  0,  $1);              }
         | COUNTS                                     { $$ = make_threshold(false, $1, 0.0);              }
         | SECONDS                                    { $$ = make_threshold(false, $1, 0.0);              }
         ;

Actuator : '{'         '}'         ';'                { printf("Triggers must list pins\n"); exit(1);     }
         | '{'         '}' Options ';'                { printf("Triggers must list pins\n"); exit(1);     }
         | '{' Charges '}'         ';'                { $$ = make_trigger($2, NULL);                      }
         | '{' Charges '}' Options ';'                { $$ = make_trigger($2,   $4);                      }
         ;

Charges  : CHARGE                                     { $$ = list_from(1, make_charge($1));               }
         | Charges ',' CHARGE                         { list_insert($1, make_charge($3)); $$ = $1;        }
         ;

Options  : ID                                         { $$ = list_from(1, $1);                            }
         | Options ',' ID                             { list_insert($1, $3); $$ = $1;                     }
         ;

%%

Charge * make_charge(int raw_charge) {
  // note, pin 0 not allowed. Shouldn't matter since that's SDA.0,
  // but it's important for future bug potential
  
  Charge * charge = malloc(sizeof(Charge));
  
  charge -> gpio = raw_charge;
  
  charge -> hot = (raw_charge > 0);
  if (!charge -> hot) charge -> gpio *= -1;
  
  return charge;
}

Threshold * make_threshold(bool use_decimal, int integer, double decimal) {
  
  Threshold * threshold = malloc(sizeof(Threshold));
  
  threshold -> use_decimal = use_decimal;
  
  if (use_decimal) threshold -> decimal = decimal;
  else             threshold -> integer = integer;
  
  return threshold;
}

Trigger * make_trigger(List * charges, List * options) {
  // creates a trigger, which gets modified later
  
  Trigger * trigger = malloc(sizeof(Trigger));
  
  trigger -> fired    = false;
  trigger -> singular = true;     // defaults
  trigger -> reverses = false;    // --------
  
  trigger -> charges = charges;
  
  if (!options) return trigger;
  
  for (iterate(options, char *, option)) {
    if      (!strcmp(option, "singular")) trigger -> singular = true;
    else if (!strcmp(option, "forever" )) trigger -> singular = false;
    else if (!strcmp(option, "reverses")) trigger -> reverses = true;
    else {
      printf("Unknown option " RED "%s\n" RESET, option);
      exit(1);
    }
  }
  
  return trigger;
}

Trigger * modify_trigger(char * id, bool less, Threshold * threshold, Trigger * trigger) {
  
  trigger -> id = id;
  trigger -> less = less;
  
  if (threshold -> use_decimal) trigger -> threshold.decimal = threshold -> decimal;
  else                          trigger -> threshold.integer = threshold -> integer;
  
  free(threshold);
  
  return trigger;
}

			 
void build_sensor(char * id, int hertz, List * triggers, bool print) {
  
  ProtoSensor * proto = hashmap_get(proto_sensors, id);
  
  if (!proto) {
    printf(RED "%s is not a sensor\n" RESET, id);
    exit(1);
  }
  
  proto -> hertz = hertz;
  proto -> print = print;
  proto -> triggers = triggers;
  proto -> requested = true;

  if (!triggers) return;

  // duplicate reversing triggers
  for (iterate(triggers, Trigger *, trigger)) {
    if (trigger -> reverses) {
      
      Trigger * opposite = malloc(sizeof(Trigger));

      opposite -> id       =  trigger -> id;
      opposite -> less     = !trigger -> less;
      opposite -> fired    =  trigger -> fired;
      opposite -> singular =  trigger -> singular;
      opposite -> reverses =  false;//trigger -> reverses;
      
      opposite -> threshold.decimal = trigger -> threshold.decimal;
      opposite -> charges = list_create();
      
      for (iterate(trigger -> charges, Charge *, charge)) {
	
	Charge * anti_charge = malloc(sizeof(Charge));
	
	anti_charge -> gpio =  charge -> gpio;
	anti_charge -> hot  = !charge -> hot;
	
	list_insert(opposite -> charges, anti_charge);
      }
      
      list_insert(triggers, opposite);
      //trigger_index++;
    }
  }
  
  for (iterate(triggers, Trigger *, trigger)) {
    
    if (!proto -> targets || !hashmap_exists(proto -> targets, trigger -> id)) {
      printf(RED "Trigger target %s unknown\n" RESET, trigger -> id);
      exit(1);
    }
  }
}

void print_config() {
  
  printf("\n\nTriggers\n\n");
  
  for (iterate(proto_sensors -> all, ProtoSensor *, proto)) {

    if (!proto -> requested) continue;
    if (!proto -> triggers ) continue;
    
    printf(GREEN "%s\n" RESET, proto -> code_name);
    
    for (iterate(proto -> triggers, Trigger *, trigger)) {
      
      printf(CYAN "    %s" RESET, trigger -> id);
      
      if (trigger -> less) printf(GRAY " <");
      else                 printf(GRAY " >");


      
      printf(" { " RESET);
      
      for (iterate(trigger -> charges, Charge *, charge)) {

	if (charge -> hot) printf(YELLOW "+" RESET);
	else               printf(YELLOW "-" RESET);
	
	printf("%d ", charge -> gpio);
      }

      if (!trigger -> singular) printf(GRAY "}" YELLOW " *\n" RESET);
      else                      printf(GRAY "}\n");
    }

    printf("\n");
  }
}
