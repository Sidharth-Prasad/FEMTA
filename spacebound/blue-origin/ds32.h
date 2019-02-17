#ifndef HEADER_GAURD_DS32
#define HEADER_GAURD_DS32

#include "sensor.h"
#include "i2c.h"

char formatted_time[32];

Sensor * init_ds32();

void set_time_ds32(Sensor * ds32);

#endif
