
#ifndef HEADER_GAURD_ADXL
#define HEADER_GAURD_ADXL

/**
 * Sensor:
 *   Accelerometer
 *   ADS1115
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
#include "i2c.h"
#include "types.h"

Sensor * init_ad15(uchar address);

#endif
