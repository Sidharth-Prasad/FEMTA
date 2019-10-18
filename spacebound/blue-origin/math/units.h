#ifndef HEADER_GAURD_UNITS
#define HEADER_GAURD_UNITS

#include <stdbool.h>

#include "../sensors/sensor.h"
#include "../structures/list.h"

typedef float (* Conversion)(float value);

typedef struct PathElement {
  
  bool universal;
  
  union {
    List * calibration;       // if sensor specific
    Conversion conversion;    // if universal conversion (Ex: Celcius to Fahrenheit)
  };
  
} PathElement;

void init_units();
void drop_units();
void print_units_supported();

Conversion get_universal_conversion(char * from, char * to);

PathElement * path_element_from_conversion(Conversion conversion);
PathElement * path_element_from_calibration(List * calibration);

float path_compute(List * path, float x);

#endif
