#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_TEMPERATURE_MONITORING
#define HEADER_GAURD_FOR_COMPILER_UNIT_TEMPERATURE_MONITORING

#include <stdbool.h>

#include "graphics.h"

bool initialize_temperature_monitoring();
void terminate_temperature_monitoring();

Plot * temperature_plot;

#endif
