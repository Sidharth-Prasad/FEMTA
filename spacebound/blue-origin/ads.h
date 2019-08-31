#ifndef HEADER_GAURD_ADS
#define HEADER_GAURD_ADS


#include "i2c.h"
#include "sensor.h"
#include "types.h"

#define ADS_ADDRESS_GND 0x48    // The address changes based on which
#define ADS_ADDRESS_VDD 0x49    // pin the ADS addr pin is bound to.
#define ADS_ADDRESS_SDA 0x50    // For example, binding the pin to GND
#define ADS_ADDRESS_SCL 0x51    // makes its address 0x48.


Sensor * init_ads(uint8 address, Sensor * s0, Sensor * s1, Sensor * s2, Sensor * s3);
Sensor * init_noise();    // creates a subsensor that reads raw ads values


#endif
