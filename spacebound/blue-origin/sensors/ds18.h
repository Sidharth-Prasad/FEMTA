#ifndef HEADER_GAURD_DS18
#define HEADER_GAURD_DS18

/**
 * Sensor:
 *   Low Frequency Temperature Probe
 *   DS18B20
 *   Connected via 1-wire
 * 
 * Datasheet: 
 *   https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 * 
 * License:
 *   GPLv3   
 * 
 * Author: 
 *   Noah Franks
 */

#include "sensor.h"

#include "../system/one.h"

Sensor * init_ds18(ProtoSensor * proto);


#endif

