#ifndef HEADER_GAURD_DS32
#define HEADER_GAURD_DS32

/**
 * Sensor:
 *   Real Time Clock with Temperature-compensated Crystal Oscillator
 *   RTC TCXO DS3231N
 *   Connected via I2C
 * 
 * Datasheet: 
 *   https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
 * 
 * License:
 *   GPLv3   
 * 
 * Author: 
 *   Noah Franks
 */


#include "sensor.h"

#include "../system/i2c.h"

#define DS32_MEASURE_TIME        0
#define DS32_MEASURE_TEMPERATURE 1

#define DS32_ADDRESS 0x68

int initial_seconds;
char formatted_time[32];
long experiment_start_time;

Sensor * init_ds32(Sensor * proto);

void set_time_ds32(Sensor * ds32);

#endif
