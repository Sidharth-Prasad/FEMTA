#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include "list.h"
#include "i2c.h"
#include "types.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;

typedef void (* sensor_free)(Sensor * sensor);


typedef struct Sensor {
  
  char * name;
  

  union {
    i2c_device * i2c;
  };
  
  sensor_free free;
  
} Sensor;

List * sensors;

void init_sensors();


#endif
