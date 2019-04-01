#ifndef HEADER_GAURD_AD15
#define HEADER_GAURD_AD15

/**
 * Sensor:
 *   Accelerometer
 *   ADS1115
 *   Connected via I2C
 * 
 * Datasheet: 
 *   https://cdn-shop.adafruit.com/datasheets/ads1115.pdf
 * 
 * License:
 *   GPLv3   
 * 
 * Author: 
 *   Noah Franks
 */

#include "sensor.h"

#include "../system/i2c.h"
#include "../types/types.h"
#include "../structures/list.h"

#define A01 0b000
#define A23 0b011
#define A0  0b100
#define A1  0b101
#define A2  0b110
#define A3  0b111

#define AD15_GND 0x48
#define AD15_VDD 0x49
#define AD15_SDA 0x4A
#define AD15_SCL 0x4B

Sensor * init_ad15(uint8 address, char * title, List * modes, List * names);

typedef struct AD15_Config {
  // read pages 18-19 of datasheet for more informaiton;
  // some of this has a bit of nuance.

  union {
    uchar low_byte;          // ease of byte access
    struct {
      uchar COMP_QUE : 2;    // number of conversions needed to trigger comparator
      uchar COMP_LAT : 1;    // whether to latch the comparator
      uchar COMP_POL : 1;    // polarity of the comparator
      uchar COMP_MODE: 1;    // type of comparator to employ
      uchar DATA_RATE: 3;    // samples per second
    };
  };

  union {
    uchar high_byte;         // ease of byte access
    struct {
      uchar MODE: 1;         // high or low-power state
      uchar PGA : 3;         // programmable gain amplifier
      uchar MUX : 3;         // multiplexer setup
      uchar OS  : 1;         // sleep state
    };
  };

  List * modes;    // list of modes used to change config as needed
  
} AD15_Config;

#endif

