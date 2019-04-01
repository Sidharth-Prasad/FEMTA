#ifndef HEADER_GAURD_ADXL
#define HEADER_GAURD_ADXL

/**
 * Sensor:
 *   Accelerometer
 *   ADXL345
 *   Connected via I2C
 * 
 * Datasheet: 
 *   https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf
 * 
 * License:
 *   GPLv3   
 * 
 * Author: 
 *   Noah Franks
 */


#include "sensor.h"

#include "../system/i2c.h"

Sensor * init_adxl();

#endif
