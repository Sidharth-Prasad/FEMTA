#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include <stdio.h>

#include "list.h"
#include "i2c.h"
#include "types.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;

typedef void (* sensor_free)(Sensor * sensor);


typedef struct Sensor {
  
  char * name;           // component name
  FILE * file;           // log file
  char * buffer;         // buffer for file I/O
  
  union {
    i2c_device * i2c;    // i2c communications info
  };
  
  sensor_free free;      // how to free sensor
  
} Sensor;

List * sensors;          // every sensor on craft

void init_sensors();


#endif
