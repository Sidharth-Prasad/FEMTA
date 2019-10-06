
%{
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../math/units.h"
#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/color.h"
#include "../system/error.h"
#include "../sensors/sensor.h"

extern FILE * yyin;

int yylex();
void yyerror(char * message);

typedef struct Trigger Trigger;
typedef struct Numeric Numeric;
typedef struct Specification Specification;

Charge        * make_charge(Numeric * wire, Specification * tag);
Trigger       * make_trigger(List * charges, List * options);
Specification * specify_trigger(char * id, bool less, Numeric * threshold, Trigger * trigger);
Specification * make_tag(char * id, List * options, List * args);

void build_sensor(char * id, Numeric * frequency, Numeric * denominator, List * specifications);
void print_config();
%}

%code requires {
  
  #include <stdbool.h>
  #include "../math/units.h"
  #include "../structures/list.h"
  #include "../sensors/sensor.h"
  
  typedef struct Specification {
      
      char * id;
      List * options;
      List * args;
      
      bool should_destroy_options;
      bool should_destroy_args;
      
  } Specification;
}

%union {
  char          * string;
  int           * integer;
  float         * decimal;
  Numeric       * numeric;
  
  List          * list;
  Charge        * charge;
  Trigger       * trigger;
  Specification * specification;
}

%token TRIGGER

%token  <string>        ID
%token  <numeric>       NUMERIC
                        
%type   <list>          Specs Charges Args Options
%type   <charge>        Charge
%type   <trigger>       Actuator
%type   <specification> Spec Tag
                        
%start Config

                        /* Notes:                                             *
                         *                                                    *
                         * Specifications include triggers and tags.          *
                         * Tags may be destroyed when their specification is  *
                         * interpreted, as is the case whith make_charge()    *
                         *                                                    *
                         * Units must never exceed 7 characters.              *
                         * Additionally, two generic units exist: i and f     *
                         *   i - the generic 32-bit integer                   *
                         *   f - the generic 32-bit floating point number     *
                         *                                                    *
                         */

%%

Config   : Sensors
         |                                            { printf("\nExperiment does not involve sensors"); }
         ;

Sensors  : Sensor                
         | Sensors Sensor
         ;

Sensor   : ID NUMERIC             '{'       '}' ';'   { build_sensor($1, $2, NULL, NULL);                }
         | ID NUMERIC             '{' Specs '}' ';'   { build_sensor($1, $2, NULL,   $4);                }
         | ID NUMERIC '/' NUMERIC '{'       '}' ';'   { build_sensor($1, $2,   $4, NULL);                }
         | ID NUMERIC '/' NUMERIC '{' Specs '}' ';'   { build_sensor($1, $2,   $4,   $6);                }
         ;

Specs    : Spec                                       { $$ = list_from(1, $1);                           }
         | Specs Spec                                 { list_insert($1, $2); $$ = $1;                    }
         ;

Spec     : ID '<' NUMERIC TRIGGER Actuator            { $$ = specify_trigger($1,  true, $3, $5);         }
         | ID '>' NUMERIC TRIGGER Actuator            { $$ = specify_trigger($1, false, $3, $5);         }
         | Tag ';'                                    { $$ = $1;                                         }
         ;

Actuator : '{'         '}'         ';'                { printf("Triggers must list pins\n"); exit(3);    }
         | '{'         '}' Options ';'                { printf("Triggers must list pins\n"); exit(3);    }
         | '{' Charges '}'         ';'                { $$ = make_trigger($2, NULL);                     }
         | '{' Charges '}' Options ';'                { $$ = make_trigger($2,   $4);                     }
         ;

Charges  : Charge                                     { $$ = list_from(1, $1);                           }
         | Charges ',' Charge                         { list_insert($1, $3); $$ = $1;                    }
         ;

Charge   : NUMERIC Tag                                { $$ = make_charge($1,   $2);                      }
         | NUMERIC                                    { $$ = make_charge($1, NULL);                      }
         ;

Tag      : '[' ID                      ']'            { $$ = make_tag($2, NULL, NULL);                   }
         | '[' ID ':'         ':' Args ']'            { $$ = make_tag($2, NULL,   $5);                   }
         | '[' ID ':' Options ':' Args ']'            { $$ = make_tag($2,   $4,   $6);                   }
         ;

Args     : Args ',' NUMERIC                           { list_insert($1, $3); $$ = $1;                    }
         | NUMERIC                                    { $$ = list_from(1, $1);                           }
         ;

Options  : ID                                         { $$ = list_from(1, $1);                           }
         | Options ',' ID                             { list_insert($1, $3); $$ = $1;                    }
         ;

%%

static void represent_as_decimal(Numeric * numeric) {
    
    if (numeric -> is_decimal) return;
    
    numeric -> decimal    = (float) numeric -> integer;
    numeric -> is_decimal = true;
    
    if (!strcmp(numeric -> units, "i"))    // change generic unit if needed
      numeric -> units[0] = 'f';           // -----------------------------
}

static void specification_destroy(void * vspecification) {
    
    Specification * specification = vspecification;
    
    /*if (specification -> options && specification -> should_destroy_options)
        list_destroy(specification -> options);
    
    if (specification -> args && specification -> should_destroy_args)
    list_destroy(specification -> args);*/
    
    free(specification -> id);
    free(specification);
}

Charge * make_charge(Numeric * wire, Specification * tag) {
    /* note, pin 0 not allowed. Shouldn't matter since that's SDA.0, *
     * but it's important for future bug potential.                  */
  
    if (strcmp(wire -> units, "i")) {
        printf(RED "Charge wires must be integers" RESET);
	exit(ERROR_EXPERIMENTER);        
    }
    
    Charge * charge = calloc(1, sizeof(*charge));
    
    charge -> gpio = abs(wire -> integer);
    charge -> hot  = (wire -> integer > 0);
    
    if (tag) {
        if (strcmp(tag -> id, "pulse")) {
            printf(RED "Unknown tag %s for charge\n" RESET, tag -> id);
            exit(ERROR_EXPERIMENTER);
        }
        
        if (tag -> args -> size != 1) {
            printf(RED "Exactly 1 argument shoud be used for 'pulse' tag\n" RESET);
            exit(ERROR_EXPERIMENTER);
        }
        
        charge -> duration = (int) tag -> args -> head -> value;
        
        specification_destroy(tag);    // no longer needed
    }
    
    return charge;
}

Trigger * make_trigger(List * charges, List * options) {
    // creates a trigger, which becomes a specification later
    
    Trigger * trigger = calloc(1, sizeof(*trigger));
    
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
            exit(ERROR_EXPERIMENTER);
        }
    }
    
    return trigger;
}


Specification * specify_trigger(char * id, bool less, Numeric * threshold, Trigger * trigger) {
    /* Wraps and modifies the trigger into a specification so that sensor construction *
     * may have a linked list consisting of nodes with the same content                */
    
    represent_as_decimal(threshold);
    
    trigger -> id        = id;
    trigger -> less      = less;
    trigger -> threshold = threshold;
    
    Specification * specification = malloc(sizeof(*specification));
    
    specification -> id      = strdup("trigger");
    specification -> options = list_from(1, trigger);
    
    return specification;
}

Specification * make_tag(char * id, List * options, List * args) {
  
    Specification * tag = malloc(sizeof(*tag));
    
    tag -> id      = id;
    tag -> options = options;
    tag -> args    = args;
    
    
    // [ print, color : 5hz ] - no color or freq required
    // [ smooth : ] -
    // [ calibrate target : 1.0, 3.4, ... ]
    
    
    
    /* Perform error checking on this tag now */
    
    if (!strcmp(id, "print")) {
      
        if (options) {
	  
            char * color      = (char *) options -> head -> value;
            char * color_code = get_color_by_name(color);
            
            if (!color_code) {
                printf(RED "Color name %s not recognized\n" RESET, color);
                exit(ERROR_EXPERIMENTER);
            }
        }
    }
    
    else if (!strcmp(id, "pulse")) {
        
        if (options)
            exit_printing("Pulsing does not support options at this time\n", ERROR_EXPERIMENTER);
	
        if (!args || args -> size != 1)
            exit_printing("Pulsing requires exactly 1 argument\n", ERROR_EXPERIMENTER);
    }
    
    else if (!strcmp(id, "smooth")) {
      
        if (options)
            exit_printing("Smoothing does not support options at this time\n", ERROR_EXPERIMENTER);
	
        if (args) {
	    Numeric * numeric = (Numeric *) list_get(args, 0);
            float value = numeric -> decimal;
	    
            if (strcmp(numeric -> units, "f") || value < 0.0f || value > 1.0f) {
                printf(RED "Autoregressive smoothing requires a # in [0.0, 1.0]\n" RESET);
                exit(ERROR_EXPERIMENTER);
            }
        }
    }
    
    else if (!strcmp(id, "calibrate")) {
      
        if (!args   ) exit_printing("Calibration curves require at least one constant\n", ERROR_EXPERIMENTER);
        if (!options) exit_printing("Calibration requires a target\n", ERROR_EXPERIMENTER);
	if (options -> size < 2) exit_printing("Calibration requires a unit\n", ERROR_EXPERIMENTER);
	
        char * curve = (char *) list_get(options, 1);
        
        if      (!strcmp(curve, "poly"));
        else if (!strcmp(curve, "hart")) {
            if (args -> size != 3)
	        exit_printing("The Steinhart and Hart Equation requires 3 constants\n", ERROR_EXPERIMENTER);
        }
        else {
            printf(RED "Unknown calibration curve " CYAN "%s\n" RESET, curve);
            exit(ERROR_EXPERIMENTER);
        }
    }
    
    else {
        printf(RED "Invalid tag id " CYAN "%s\n" RESET, id);
        exit(ERROR_EXPERIMENTER);
    }
    
    return tag;
}

void build_sensor(char * id, Numeric * frequency, Numeric * denominator, List * specifications) {
  
  ProtoSensor * proto = hashmap_get(proto_sensors, id);
  
  if (!proto) {
    printf(RED "%s is not a sensor\n" RESET, id);
    exit(ERROR_EXPERIMENTER);
  }
  
  if ((               strcmp(frequency   -> units, "Hz") && strcmp(frequency   -> units, "i")) ||
      (denominator && strcmp(denominator -> units, "Hz") && strcmp(denominator -> units, "i")))
    exit_printing("Sensor frequency uses bad units", ERROR_EXPERIMENTER);
  
  proto -> hertz = frequency -> integer;    // base Hz for scheduling
  
  if (denominator) proto -> hertz_denominator = denominator -> integer;
  else             proto -> hertz_denominator = 0;
  
  proto -> requested = true;
  
  if (!specifications) return;
  
  List * triggers = list_create();
  
  for (iterate(specifications, Specification *, specification)) {
      
      if (!strcmp(specification -> id, "trigger")) {
	  list_insert(triggers, list_get(specification -> options, 0));
      }
      
      else if (!strcmp(specification -> id, "print")) {
          proto -> print = true;
      }
      
      else if (!strcmp(specification -> id, "smooth")) {
	  
          Numeric * numeric = list_get(specification -> args, 0);
	  
          float auto_regressive = numeric -> decimal;
          
          if (proto -> auto_regressive) {
              printf(RED "Duplicate autoregressive constant for %s\n" RESET, id);
              exit(ERROR_EXPERIMENTER);
          }
          
          proto -> auto_regressive = auto_regressive;
      }
      
      else if (!strcmp(specification -> id, "calibrate")) {
          
          char * target = list_get(specification -> options, 0);
	  char * curve  = list_get(specification -> options, 1);
	  char * unit   = list_get(specification -> options, 2);
	  
          if (!proto -> targets || !hashmap_exists(proto -> targets, target)) {
              printf(RED "Calibration target %s for %s is not known\n" RESET, target, id);
              exit(ERROR_EXPERIMENTER);
          }
	  
          if (hashmap_exists(proto -> calibrations, target)) {
              printf(RED "Duplicate target calibration for %s of %s found\n" RESET, target, id);
              exit(ERROR_EXPERIMENTER);
          }
	  
	  // force all calibration constants to be decimals
	  for (iterate(specification -> args, Numeric *, constant))
	    represent_as_decimal(constant);
	  
	  // make first node the calibration type
	  list_insert_first(specification -> args, curve);
	  
	  hashmap_add(proto -> calibrations, target, specification -> args);
	  hashmap_add(proto -> output_units, target,                  unit);
      }
  }
  
  
  
  for (iterate(triggers, Trigger *, trigger)) {
      
      // check trigger targets
      if (!proto -> targets || !hashmap_exists(proto -> targets, trigger -> id)) {
          printf(RED "Trigger target %s unknown\n" RESET, trigger -> id);
          exit(ERROR_EXPERIMENTER);
      }
      
      // duplicate reversing triggers
      
      if (trigger -> reverses) {
	
	  Trigger * opposite = calloc(1, sizeof(*opposite));
	  
	  opposite -> id       =  strdup(trigger -> id);
	  opposite -> less     = !trigger -> less;
	  opposite -> fired    =  trigger -> fired;
	  opposite -> singular =  trigger -> singular;
	  opposite -> reverses =  false;
	  	  
	  opposite -> threshold = trigger -> threshold;
	  opposite -> charges   = list_create();
	  
	  for (iterate(trigger -> charges, Charge *, charge)) {
	      
              Charge * anti_charge = malloc(sizeof(*charge));
	      
	      anti_charge -> gpio     =  charge -> gpio;
	      anti_charge -> hot      = !charge -> hot;
	      anti_charge -> duration =  charge -> duration;
	      
	      list_insert(opposite -> charges, anti_charge);
	  }
	  
	  list_insert(triggers, opposite);
      }
  }
  
  proto -> triggers = triggers;
  
  specifications -> free = specification_destroy;
  list_destroy(specifications);
  
  n_triggers += triggers -> size;
}

void print_config() {
  
  if (!n_triggers) {
    printf("\n\nExperiment does not involve triggers\n\n");
    return;
  }
  
  printf("\n\n" GRAY "%d" RESET " Triggers\n\n", n_triggers);
  
  for (iterate(proto_sensors -> all, ProtoSensor *, proto)) {
    
    if (!proto -> requested) continue;
    if (!proto -> triggers ) continue;
    
    printf(GREEN "%s\n" RESET, proto -> code_name);
    
    for (iterate(proto -> triggers, Trigger *, trigger)) {
      
      Numeric * numeric = trigger -> threshold;
      
      Numeric standard;
      to_standard_units(&standard, numeric);
      
      printf(CYAN "    %s" RESET, trigger -> id);
      
      if (trigger -> less)
	printf(GRAY " <" MAGENTA " %.3f%s " GRAY "(= " MAGENTA "%.3f%s" GRAY ")",
	       numeric -> decimal, numeric -> units, standard.decimal, standard.units);
      else
	printf(GRAY " >" MAGENTA " %.3f%s " GRAY "(= " MAGENTA "%.3f%s" GRAY ")",
	       numeric -> decimal, numeric -> units, standard.decimal, standard.units);
      
      
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
