#ifndef HEADER_GAURD_FRAM
#define HEADER_GAURD_FRAM

#include "sensor.h"
#include "i2c.h"

/**
 * Sensor:
 *   Ferroelectric Random Access Memory
 *   FRAM MB85RC256V
 *   Connected via I2C
 * 
 * Datasheet: 
 *   https://cdn-learn.adafruit.com/assets/assets/000/043/904/original/MB85RC256V-DS501-00017-3v0-E.pdf?1500009796
 * 
 * License:
 *   GPLv3   
 * 
 * Author: 
 *   Noah Franks
 */


Sensor * init_fram();

#endif
