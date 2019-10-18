
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "units.h"
#include "mathematics.h"

#include "../sensors/sensor.h"
#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/color.h"
#include "../system/compiler.h"
#include "../system/error.h"

Hashmap * conversions;

#define take(FROM, TO) convert##_##FROM##_##TO
#define arrow(FROM, TO) #FROM "->" #TO

// temperature
float take(   C,    K) (float x) { return x + 273.15f;                           }
float take(   K,    C) (float x) { return x - 273.15f;                           }
float take(   F,    C) (float x) { return x * (9.0f / 5.0f) + 32.0f;             }
float take(   C,    F) (float x) { return (x - 32.0f) * (5.0f / 9.0f);           }
float take(   K,    F) (float x) { return (x - 273.15f) * (9.0f / 5.0f) + 32.0f; }
float take(   F,    K) (float x) { return (x - 32.0f) * (5.0f / 9.0f) + 273.15f; }

// pressure
float take( atm,  kPa) (float x) { return x * 101.325f;                          }
float take( kPa,  atm) (float x) { return x / 101.325f;                          }
float take( atm, torr) (float x) { return x * 760.0f;                            }
float take(torr,  atm) (float x) { return x / 760.0f;                            }
float take( kPa, torr) (float x) { return x * 7.50062f;                          }
float take(torr,  kPa) (float x) { return x / 7.50062f;                          }

// voltage
float take(   V,   mV) (float x) { return x / 1000.0f;                           }
float take(  mV,    V) (float x) { return x * 1000.0f;                           }


void init_units() {
  
  conversions = hashmap_create(hash_string, compare_strings, NULL, 16);
  
  hashmap_add(conversions, arrow(   C,    K), take(   C,    K));
  hashmap_add(conversions, arrow(   K,    C), take(   K,    C));
  hashmap_add(conversions, arrow(   F,    C), take(   F,    C));
  hashmap_add(conversions, arrow(   C,    F), take(   C,    F));
  hashmap_add(conversions, arrow(   K,    F), take(   K,    F));
  hashmap_add(conversions, arrow(   F,    K), take(   F,    K));
  hashmap_add(conversions, arrow( atm,  kPa), take( atm,  kPa));
  hashmap_add(conversions, arrow( kPa,  atm), take( kPa,  atm));
  hashmap_add(conversions, arrow( atm, torr), take( atm, torr));
  hashmap_add(conversions, arrow(torr,  atm), take(torr,  atm));
  hashmap_add(conversions, arrow( kPa, torr), take( kPa, torr));
  hashmap_add(conversions, arrow(torr,  kPa), take(torr,  kPa));
  hashmap_add(conversions, arrow(   V,   mV), take(   V,   mV));
  hashmap_add(conversions, arrow(  mV,    V), take(  mV,    V));
}

void drop_units() {
  hashmap_destroy(conversions);
}

void print_units_supported() {
  printf
    ("Temperature\n"
     "  C    : Celcius\n"
     "  K    : Kelvin\n"
     "  F    : Fahrenheit\n"
     "\n\n"
     "Pressure\n"
     "  atm  : Atmospheres\n"
     "  kPa  : kilo-Pascals\n"
     "  torr : Torrecelli's unit\n"
     "\n\n"
     "Voltage\n"
     "   V   : Volts\n"
     "  mV   : milli-Volts\n"
     "\n"
     );
}


PathElement * path_element_from_conversion(Conversion conversion) {
  
  PathElement * element = malloc(sizeof(*element));
  
  element -> universal = true;
  element -> conversion = conversion;
  
  return element;
}

PathElement * path_element_from_calibration(List * calibration) {
  
  PathElement * element = malloc(sizeof(*element));
  
  element -> universal = false;
  element -> calibration = calibration;
  
  return element;
}


Conversion get_universal_conversion(char * from, char * to) {
  /* yields the conversion from one unit to the other, assuming the conversion
   * is the same across all domains. In other words, specific conversions related to
   * calibrations on sensors won't be returned; only generic ones like Celcius to Fahrenheit */
  
  char lookup[16];
  sprintf(lookup, "%s->%s", from, to);
  
  Conversion conversion = hashmap_get(conversions, lookup);
  
  if (unlikely(conversion == NULL)) {
    print_units_supported();
    printf("Unknown conversion %s -> %s\n", from, to);
    printf("Please use units from the table above\n");
    exit(ERROR_EXPERIMENTER);
  }
  
  return conversion;
}

static float compute_curve(float x, List * calibration) {
  
  char * curve = list_get(calibration, 0);
  
  // a calibration consists of a curve name followed by constants
  
  if (!strcmp(curve, "poly")) {
    
    // Evaluate the polynomial via Horner's Method
    
    Numeric * first = list_get(calibration, 1);
    
    float result = first -> decimal;
    
    for (iterate(calibration, Numeric *, coefficient)) {
      
      if ((int) coefficient_index < 2) continue;    // skip first 2 nodes
      
      result = result * x + coefficient -> decimal;
    }
    
    return result;
  }
  
  else if (!strcmp(curve, "hart")) {
    float A = ((Numeric *) list_get(calibration, 1)) -> decimal;
    float B = ((Numeric *) list_get(calibration, 2)) -> decimal;
    float C = ((Numeric *) list_get(calibration, 3)) -> decimal;
    
    float log_x = log(x);
    
    return 1.0f / (A + B * log_x + C * cube(log_x));
  }
  
  printf(RED "Tried to evaluate unknown curve " CYAN "%s\n" RESET);
  exit(ERROR_PROGRAMMER);
  return 0;
}

float path_compute(List * path, float x) {
  // compute a series of conversions to get a final value.
  // the series may include universal conversions as well as sensor-specific ones.
  
  for (iterate(path, PathElement *, step))
    if (step -> universal) x = (step -> convert)(x);
    else                   x = compute_curve(x, step -> calibration);
  
  return x;
}
