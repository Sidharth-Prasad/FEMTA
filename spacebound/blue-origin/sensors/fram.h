#ifndef HEADER_GAURD_FRAM
#define HEADER_GAURD_FRAM

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

#include "sensor.h"

#include "../system/i2c.h"

Sensor * init_fram();

#endif
