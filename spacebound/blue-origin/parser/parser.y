
%{
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../math/mathematics.h"
#include "../math/units.h"
#include "../sensors/sensor.h"
#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/color.h"
#include "../system/error.h"
#include "../system/gpio.h"
#include "../system/state.h"


extern FILE * yyin;

int  yylex();
void yyerror(char * message);

typedef struct Numeric Numeric;
typedef struct EffectNode EffectNode;
typedef struct Specification Specification;
typedef struct Trigger Trigger;

EffectNode    * make_charge(Numeric * wire, bool hot);
EffectNode    * make_transition(char * state_name, bool entering);
EffectNode    * add_delay(EffectNode * effect, Numeric * delay);
Trigger       * make_trigger(List * effects);
Specification * extend_trigger(List * state_names, char * id, bool less, Numeric * threshold, List * options, Trigger * trigger);
Specification * make_tag(char * id, List * options, List * args);

void build_sensor(char * id, Numeric * frequency, Numeric * denominator, List * specifications);
void print_config();
%}

%code requires {
  
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
}

%union {
  char          * string;
  int           * integer;
  float         * decimal;
  Numeric       * numeric;
  
  List          * list;
  EffectNode    * effect;
  Trigger       * trigger;
  Specification * specification;
}

%token IF SET ENTER LEAVE AFTER STATE PIN POS NEG DEFINE

%token  <string>        ID
%token  <numeric>       NUMERIC
                        
%type   <list>          Specs Args Options Effects
%type   <trigger>       Actuator
%type   <effect>        Effect
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

Config   : Defs Sensors
         | Defs                                            { printf("\nExperiment does not use sensors");         }
         ;

Defs     : Def
         | Defs Def
         ;

Def      : DEFINE LEAVE ID                                 { add_state($3, false);                                }
         | DEFINE ENTER ID                                 { add_state($3,  true);                                }
         ;

Sensors  : Sensor                
         | Sensors Sensor
         ;

Sensor   : ID NUMERIC             '{'       '}'            { build_sensor($1, $2,      NULL,  NULL);              }
         | ID NUMERIC             '{' Specs '}'            { build_sensor($1, $2,      NULL,    $4);              }
         | ID NUMERIC '/' NUMERIC '{'       '}'            { build_sensor($1, $2, $4, NULL);                      }
         | ID NUMERIC '/' NUMERIC '{' Specs '}'            { build_sensor($1, $2, $4,   $6);                      }
         ;

Specs    : Spec                                            { $$ = list_from(1, $1);                               }
         | Specs Spec                                      { list_insert($1, $2); $$ = $1;                        }
         ;

Spec     : IF '(' ID '<' NUMERIC             ')' Actuator  { $$ = extend_trigger(NULL, $3,  true, $5, NULL, $7);  }
         | IF '(' ID '<' NUMERIC ':' Options ')' Actuator  { $$ = extend_trigger(NULL, $3,  true, $5,   $7, $9);  }
         | IF '(' ID '>' NUMERIC             ')' Actuator  { $$ = extend_trigger(NULL, $3, false, $5, NULL, $7);  }
         | IF '(' ID '>' NUMERIC ':' Options ')' Actuator  { $$ = extend_trigger(NULL, $3, false, $5,   $7, $9);  }

         | IF '(' STATE Options ':' ID '<' NUMERIC             ')' Actuator
    	      { $$ = extend_trigger(  $4, $6,  true, $8, NULL, $10); }

         | IF '(' STATE Options ':' ID '<' NUMERIC ':' Options ')' Actuator
	      { $$ = extend_trigger(  $4, $6,  true, $8,  $10, $12); }

         | IF '(' STATE Options ':' ID '>' NUMERIC             ')' Actuator
	      { $$ = extend_trigger(  $4, $6, false, $8, NULL, $10); }

         | IF '(' STATE Options ':' ID '>' NUMERIC ':' Options ')' Actuator
	      { $$ = extend_trigger(  $4, $6, false, $8,  $10, $12); }

         | Tag                                             { $$ = $1;                                             }
         ;

Actuator : '{'         '}'                                 { yyerror("Empty if body");                            }
         | '{' Effects '}'                                 { $$ = make_trigger($2);                               }
         ;

Effects  : Effect                                          { $$ = list_from(1, $1);                               }
         | Effect AFTER NUMERIC                            { $$ = list_from(1, add_delay($1, $3));                }
         | Effects Effect                                  { list_insert($1, $2); $$ = $1;                        }
         | Effects Effect AFTER NUMERIC                    { list_insert($1, add_delay($2, $4)); $$ = $1;         }
         ;

Effect   : SET PIN NUMERIC POS                             { $$ = make_charge($3,  true);                         }
         | SET PIN NUMERIC NEG                             { $$ = make_charge($3, false);                         }
         | ENTER ID                                        { $$ = make_transition($2,  true);                     }
         | LEAVE ID                                        { $$ = make_transition($2, false);                     }
         ;

Tag      : '[' ID                      ']'                 { $$ = make_tag($2, NULL, NULL);                       }
         | '[' ID ':'         ':' Args ']'                 { $$ = make_tag($2, NULL,   $5);                       }
         | '[' ID ':' Options ':'      ']'                 { $$ = make_tag($2,   $4, NULL);                       }
         | '[' ID ':' Options ':' Args ']'                 { $$ = make_tag($2,   $4,   $6);                       }
         ;

Args     : Args ',' NUMERIC                                { list_insert($1, $3); $$ = $1;                        }
         | NUMERIC                                         { $$ = list_from(1, $1);                               }
         ;

Options  : ID                                              { $$ = list_from(1, $1);                               }
         | Options ',' ID                                  { list_insert($1, $3); $$ = $1;                        }
         ;

%%

static void represent_as_decimal(Numeric * numeric) {
    
    if (numeric -> is_decimal) return;
    
    numeric -> decimal    = (float) numeric -> integer;
    numeric -> is_decimal = true;
    
    if (!strcmp(numeric -> units, "i"))    // change generic unit if needed
      numeric -> units[0] = 'f';           // -----------------------------
}

static void represent_as_integer(Numeric * numeric) {
    
    if (!numeric -> is_decimal) return;
    
    numeric -> integer    = (int) numeric -> decimal;
    numeric -> is_decimal = false;
    
    if (!strcmp(numeric -> units, "f"))    // change generic unit if needed
      numeric -> units[0] = 'i';           // -----------------------------
}

static void specification_destroy(void * vspecification) {
    
    Specification * specification = vspecification;
        
    free(specification -> id);
    free(specification);
}

EffectNode * make_charge(Numeric * wire, bool hot) {
    
    if (!unit_is_of_type(wire, "Integer"))
	yyerror("Wires must be integers");
    
    if (!between(0, wire -> integer, 27)) {
        printf(RED "Broadcom " CYAN "%d " RED "does not exist\n" RESET, wire -> integer);
	yyerror("Nonexistant broadcom specified");
    }
    
    Charge * charge = calloc(1, sizeof(*charge));
    
    charge -> gpio = wire -> integer;
    charge -> hot  = hot;
    
    EffectNode * effect = calloc(1, sizeof(*effect));
    
    effect -> charge = charge;
    
    return effect;
}

EffectNode * make_transition(char * state_name, bool entering) {
  
  if (!state_exists(state_name)) {
    printf(RED "Unknown state name: " CYAN "%s\n" RESET, state_name);
    yyerror("State not found");
  }
  
  EffectNode * effect  = calloc(1, sizeof(*effect));
  
  effect -> is_charge  = false;
  effect -> state_name = state_name;
  effect -> entering   = entering;
  
  return effect;
}

EffectNode * add_delay(EffectNode * effect, Numeric * delay) {
  
  represent_as_decimal(delay);
  
  if (delay -> decimal <= 0) {
    printf(RED "Delay %f not allowed: value is non-positive" RESET, delay -> decimal);
    yyerror("Improper delay specified");
  }
  
  if (!unit_is_of_type(delay, "Time")) {
    printf("%s is not a unit of time", delay -> units);
    yyerror("Improper delay specified");
  }
  
  effect -> delay_ms = (get_universal_conversion(delay -> units, "ms"))(delay -> decimal);  
  return effect;
}

Trigger * make_trigger(List * effects) {
    // creates a trigger, which becomes a specification later
    
    Trigger * trigger = calloc(1, sizeof(*trigger));
    
    trigger -> fired    = false;
    trigger -> singular =  true;    // defaults
    trigger -> reverses = false;    // --------
    
    for (iterate(effects, EffectNode *, effect))  
      if (effect -> is_charge)
	if (effect -> charge -> hot) list_insert(trigger -> wires_high, effect -> charge    );
	else                         list_insert(trigger -> wires_low , effect -> charge    );
      else
	if (effect -> entering     ) list_insert(trigger -> enter_set , effect -> state_name);
	else                         list_insert(trigger -> leave_set , effect -> state_name);
    
    list_destroy(effects);
    return trigger;
}


Specification * extend_trigger(
			       List    * state_names,    // what conjunction precondition this trigger
			       char    * id,             // the output stream name (like A0)
			       bool      less,           // the comparison direction with the threshold
			       Numeric * threshold,      // the threshold beyond which actuation occurs
			       List    * options,        // the specific type of trigger
			       Trigger * trigger         // the trigger to be extended
			       ) {
    /* Wraps and modifies the trigger into a specification so that sensor construction *
     * may have a linked list consisting of nodes with the same content                */
    
    for (iterate(state_names, char *, state_name)) {
      if (!state_exists(state_name)) {
	printf(RED "Unknown state name " CYAN "%s\n" RESET, state_name);
	yyerror("Invalid state list for trigger");
      }
    }
    
    trigger -> precondition = state_names;
    
    
    represent_as_decimal(threshold);
    
    trigger -> id   = id;
    trigger -> less = less;
    
    trigger -> threshold_as_specified = threshold;    // this gets converted later
    
    if (options) {
        for (iterate(options, char *, option)) {
	    if      (!strcmp(option, "singular")) trigger -> singular =  true;
	    else if (!strcmp(option, "forever" )) trigger -> singular = false;
	    else if (!strcmp(option, "reverses")) trigger -> reverses =  true;
	    else {
	        printf(RED "Unknown option " CYAN "%s\n" RESET, option);
		yyerror("Unknown option specified");
	    }
	}
    }
    
    Specification * specification = calloc(1, sizeof(*specification));
    
    specification -> id      = strdup("trigger");
    specification -> options = list_from(1, trigger);
    
    return specification;
}

Specification * make_tag(char * id, List * options, List * args) {
  
    Specification * tag = calloc(1, sizeof(*tag));
    
    tag -> id      = id;
    tag -> options = options;
    tag -> args    = args;
        
    
    /* Perform preliminary error checking on this tag now */
    
    if (!strcmp(id, "print")) {
      
        if (options) {
	  
	    if (options -> size > 1) yyerror("Printing permits at most 1 option (the console color)");
	    
            char * color = list_get(options, 0);
	    
            if (!get_color_by_name(color)) {
                printf(RED "Color name %s not recognized\n" RESET, color);
		yyerror("Invalid color name specified");
            }
        }
	
	if (args) {
	  
            Numeric * numeric = list_get(args, 0);
	  
	    if (args -> size > 1               ) yyerror("Printing permits at most 1 argument (the frequency)");
	    if (!strcmp(numeric -> units, "Hz")) yyerror("Frequencies must be in Hz"                          );
	    if (numeric -> is_decimal          ) yyerror("Frequencies must be integers or rational numbers"   );
	}
    }
    
    else if (!strcmp(id, "pulse")) {        
        if (options)                    yyerror("Pulsing does not support options at this time");
        if (!args || args -> size != 1) yyerror("Pulsing requires exactly 1 argument"          );
    }
    
    else if (!strcmp(id, "smooth")) {
      
        if (options) yyerror("Smoothing does not support options at this time");
	
        if (args) {
	    Numeric * numeric = (Numeric *) list_get(args, 0);
            float value = numeric -> decimal;
	    
            if (strcmp(numeric -> units, "f") || value < 0.0f || value > 1.0f)
	      yyerror("Autoregressive smoothing requires a decimal # in [0.0, 1.0]");
        }
    }
    
    else if (!strcmp(id, "calibrate")) {
      
        if (!args   )            yyerror("Calibration curves require at least one constant");
        if (!options)            yyerror("Calibration requires a target");
	if (options -> size < 2) yyerror("Calibration requires a unit");
	
        char * curve = (char *) list_get(options, 1);
        
        if      (!strcmp(curve, "poly"));
        else if (!strcmp(curve, "hart")) {
            if (args -> size != 3)
	        yyerror("The Steinhart and Hart Equation requires exactly 3 constants");
        }
        else {
            printf(RED "Unknown calibration curve " CYAN "%s\n" RESET, curve);
            yyerror("Malformed calibration");
        }
    }
    
    else if (!strcmp(id, "conversions")) {
      
      if (args                               ) yyerror("A conversion series should never have arguments"         );
      if (!options || options -> size < 3    ) yyerror("A conversion series must include a target, then 2+ units");
      if (strcmp(list_get(options, 1), "raw")) yyerror("A conversion series always starts with 'raw'"            );
      
      for (iterate(options, char *, unit_name)) {
	
	if ((int) unit_name_index < 1) continue;    // first is target, so skip (see list.h)
	
	if (!unit_is_supported(unit_name)) {
	  printf(RED "Unknown unit name " CYAN "%s\n" RESET, unit_name);
	  yyerror("Unsupported unit provided");
	}
      }
    }
    
    else {
        printf(RED "Invalid tag id " CYAN "%s\n" RESET, id);
	yyerror("Malformed tag");
    }
    
    return tag;
}

void build_sensor(char * id, Numeric * frequency, Numeric * denominator, List * specifications) {
  
  Sensor * proto = hashmap_get(all_sensors, id);
  
  if (!proto) {
    printf(CYAN "%s " RED "is not a sensor\n" RESET, id);
    yyerror("Unknown sensor name");
  }
  
  if ((               strcmp(frequency   -> units, "Hz") && strcmp(frequency   -> units, "i")) ||
      (denominator && strcmp(denominator -> units, "Hz") && strcmp(denominator -> units, "i")))
    yyerror("Sensor frequencies must be integers or rational numbers of Hz");
  
  proto -> hertz = frequency -> integer;    // base Hz for scheduling
  
  if (denominator) proto -> hertz_denominator = denominator -> integer;
  else             proto -> hertz_denominator = 0;
  
  proto -> requested = true;
  
  for (int stream = 0; stream < proto -> data_streams; stream++)
    proto -> outputs[stream].triggers = list_create();
  
  if (!specifications) return;
  
  
  List * all_calibrations = list_create();
  
  for (iterate(specifications, Specification *, specification)) {
    
      List * options = specification -> options;
      List * args    = specification -> args;
      
      if (!strcmp(specification -> id, "trigger")) {
	
	  /* can't yet do conversion, reversals, etc,
	   * so we do the rest in a later for loop */
	  
	  Trigger * trigger = list_get(options, 0);    // we encapsulate the trigger within the options list
	  
	  // check trigger targets
	  if (!proto -> targets || !hashmap_exists(proto -> targets, trigger -> id)) {
	      printf(RED "Trigger target " CYAN "%s " RED "unknown\n" RESET, trigger -> id);
	      yyerror("Malformed trigger");
	  }
	  
	  int stream = (int) hashmap_get(proto -> targets, trigger -> id);
	  
	  list_insert(proto -> outputs[stream].triggers, trigger);
	  list_destroy(options);
      }
      
      else if (!strcmp(specification -> id, "print")) {
	  
  	  if (options) {
	      proto -> print_code = get_color_by_name(list_get(options, 0));
	      list_destroy(options);
	  }
	  
	  if (args) {
	    proto -> print_hertz = ((Numeric *) list_get(args, 0)) -> integer;
	    list_destroy(args);
	  }
	  
	  proto -> print = true;
      }
      
      else if (!strcmp(specification -> id, "smooth")) {
	  
          Numeric * numeric = list_get(args, 0);
	  
          float auto_regressive = numeric -> decimal;
          
          if (proto -> auto_regressive) {
              printf(RED "Duplicate autoregressive constant for %s\n" RESET, id);
	      yyerror("Duplicate autoregressive constant");
          }
          
          proto -> auto_regressive = auto_regressive;
	  list_destroy(args);
      }
      
      else if (!strcmp(specification -> id, "calibrate")) {
	  
	  /* as with the triggers, we can't do everything here (like forming each output series)
	   * so we'll place each calibration (which are themselves lists) into an UNORDERED list
	   * for each target using the calibrations hashmap. Later, we'll actually convert these
	   * unordered lists into proper serieses. */
	  
          char * target = list_get(options, 0);
	  char * curve  = list_get(options, 1);
	  char * unit   = list_get(options, 2);
	  
          if (!proto -> targets || !hashmap_exists(proto -> targets, target)) {
              printf(RED "Calibration target " CYAN "%s " RED "for %s is not known\n" RESET, target, id);
	      yyerror("Malformed calibration");
          }
	  
	  // force all calibration constants to be decimals
	  for (iterate(specification -> args, Numeric *, constant))
	      represent_as_decimal(constant);
	  
	  
	  Calibration * calibration = calloc(1, sizeof(*calibration));
	  
	  calibration -> curve     = curve;
	  calibration -> constants = specification -> args;
	  calibration -> target    = target;
	  
	  list_insert(all_calibrations, calibration);
      }
      
      else if (!strcmp(specification -> id, "conversions") {
	  
	  char * target = list_get(options, 0);
	  
	  if (!proto -> targets || !hashmap_exists(proto -> targets, target)) {
              printf(RED "Conversion series target " CYAN "%s " RED "for %s is not known\n" RESET, target, id);
	      yyerror("Malformed conversion series");
          }
	  
	  // START HERERE
      }
  }
    
  
  for (iterate(all_calibrations, Calibration *, calibrations)) {
    
      
  }

  list_destroy(all_calibrations);
  all_calibrations = NULL;
  
  for (iterate(proto -> triggers, Trigger *, trigger)) {
            
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
	  
	  list_insert(proto -> triggers, opposite);
      }
  }
  
  specifications -> free = specification_destroy;    // Note: up to us to destroy lists earlier
  list_destroy(specifications);                      // ---------------------------------------
  
  n_triggers += proto -> triggers -> size;
}

void print_config() {
  
  if (!n_triggers) {
    printf("\n\nExperiment does not involve triggers\n\n");
    return;
  }
  
  printf("\n\n" GRAY "%d" RESET " Triggers\n\n", n_triggers);
  
  for (iterate(proto_sensors -> all, ProtoSensor *, proto)) {
    
    if (!proto -> requested       ) continue;
    if (!proto -> triggers -> size) continue;
    
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
