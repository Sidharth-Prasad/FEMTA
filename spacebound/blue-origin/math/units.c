
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "units.h"
#include "mathematics.h"

#include "../sensors/sensor.h"
#include "../structures/list.h"
#include "../system/color.h"
#include "../system/error.h"

float to_standard_units(Numeric * dest, Numeric * source) {
  
  float as_standard = source -> decimal;
  float x           = source -> decimal;
  
  char * standard_units = "error";

  // voltz
  if (!strcmp(source -> units, "V")) {
    as_standard = x;
    standard_units = "V";
  }
  else if (!strcmp(source -> units, "mV")) {
    as_standard = x / 1000.0f;
    standard_units = "V";
  }
  
  // temperature
  else if (!strcmp(source -> units, "C")) {
    as_standard = x;
    standard_units = "C";
  }
  else if (!strcmp(source -> units, "K")) {
    as_standard = x - 273; 
    standard_units = "C";
  }
  else if (!strcmp(source -> units, "F")) {
    as_standard = (5.0f/9.0f) * (x - 32);
    standard_units = "C";
  }
  
  // time
  else if (!strcmp(source -> units, "s")) {
    as_standard = x;
    standard_units = "s";
  }
  else if (!strcmp(source -> units, "min")) {
    as_standard = x * 60.0f;
    standard_units = "s";
  }
  else if (!strcmp(source -> units, "ms")) {
    as_standard = x / 1000.0f;
    standard_units = "s";
  }
  
  dest -> decimal = as_standard;
  strcpy(dest -> units, standard_units);
  
  return as_standard;
}

float convert_to(char * units, float value) {
  return 0;
}

float compute_curve(float x, List * calibration) {
  
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
