#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include <stdio.h>

#include "../structures/list.h"
#include "../system/i2c.h"
#include "../types/types.h"

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
  
<<<<<<< HEAD:spacebound/blue-origin/sensor.h
  sensor_free free;             // how to free sensor
=======
  sensor_free free;      // how to free sensor

  void * data;           // specialized sensor data
>>>>>>> c1dd616cf6a2be8738025b48b41f17890b74995e:spacebound/blue-origin/sensors/sensor.h
  
} Sensor;

List * sensors;                 // every sensor on craft

void init_sensors();

Sensor * create_sensor(char * name, sensor_free freer);


#endif
