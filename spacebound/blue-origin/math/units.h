#ifndef HEADER_GAURD_UNITS
#define HEADER_GAURD_UNITS

#include "../sensors/sensor.h"
#include "../structures/list.h"

float to_standard_units(Numeric * dest, Numeric * source);
float convert_to(char * units, float value);

float compute_curve(float value, List * calibration);

#endif
