#ifndef HEADER_GAURD_UNITS
#define HEADER_GAURD_UNITS

#include <stdbool.h>

#include "../sensors/sensor.h"
#include "../structures/list.h"

typedef float (* Conversion)(float value);

typedef struct Numeric {
  
  union {
    int   integer;    // the numerical value 
    float decimal;    // -------------------
  };
  
  char units[8];      // the units code
  bool is_decimal;    // representation
  
} Numeric;

typedef struct Calibration {
  
  char * curve;        // the type of curve used to compute values
  List * constants;    // the constants defining the curve
  char * target;       // data stream calibration acts on
  char * unit_from;    // the unit from which to convert
  char * unit_to;      // the unit to which to convert
  
} Calibration;

typedef struct SeriesElement {
  
  bool universal;
  
  union {
    Calibration * calibration;   // if sensor specific
    Conversion    conversion;    // if universal conversion (Ex: Celcius to Fahrenheit)
  };
  
} SeriesElement;

void init_units();
void drop_units();
void print_units_supported();
bool unit_is_supported(char * unit_name);
bool unit_is_of_type(Numeric * numeric, char * type_name);

Numeric * numeric_from_decimal(float decimal);
Numeric * numeric_from_integer(float integer);

Conversion get_universal_conversion(char * from, char * to);

SeriesElement * series_element_from_conversion(Conversion conversion);
SeriesElement * series_element_from_calibration(Calibration * calibration);

float series_compute(List * series, float x);
void  series_destroy(List * series);

float convert_identity(float x);

#endif
