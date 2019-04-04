#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include <stdio.h>

#include "list.h"
#include "i2c.h"
#include "types.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;
typedef struct sub_device sub_device;

typedef void (* sensor_free)(Sensor * sensor);


typedef struct Sensor {
  
  char * name;                  // component name
  
  union {
    struct {
      i2c_device * i2c;         // i2c communications info
      Sensor * subsystem[4];    // subsystem SLL
    };
    struct {
      sub_device * sub;
    };
  };
  
  sensor_free free;             // how to free sensor
  
} Sensor;

List * sensors;                 // every sensor on craft

void init_sensors();

Sensor * create_sensor(char * name, sensor_free freer);


#endif
