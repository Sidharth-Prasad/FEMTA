#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include <stdbool.h>
#include <stdio.h>

#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/i2c.h"
#include "../types/types.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;

typedef void (* sensor_free)(Sensor * sensor);

typedef struct Trigger {
  
  char * id;
  bool less;
  
  int threshold;
  int gpio;
  
  bool fired;
  
} Trigger;


typedef struct Sensor {
  
  char * name;           // component name
  bool   print;          // whether sensor prints
  
  union {
    i2c_device * i2c;    // i2c communications info
  };
  
  List * triggers;       // sensor triggers
  Hashmap * targets;     // that which can be triggered
  
  sensor_free free;      // how to free sensor
  
  void * data;           // specialized sensor data
  
} Sensor;

typedef struct ProtoSensor {
  
  char * code_name;          // abbreviated name
  
  int hertz;                 // frequency in hertz
  uint8 address;             // i2c address
  
  List * triggers;           // gpio triggers
  Hashmap * targets;         // that which can be triggered
  
  bool print;                // whether sensor is printed to console
  bool requested;            // whether sensor is requested
  
} ProtoSensor;

List * sensors;             // every active sensor on craft

Hashmap * proto_sensors;    // sensors that could be specified

Trigger * trigger_create(char * id, bool less, int threshold, int gpio);

void init_sensors();
void start_sensors();

#endif
