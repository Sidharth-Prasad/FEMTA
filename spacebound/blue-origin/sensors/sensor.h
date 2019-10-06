#ifndef HEADER_GAURD_SENSOR
#define HEADER_GAURD_SENSOR

#include <stdbool.h>
#include <stdio.h>

#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/i2c.h"
#include "../system/one.h"
#include "../types/types.h"

#define I2C_BUS 0x1
#define ONE_BUS 0x2

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;
typedef struct one_device one_device;

typedef void (* sensor_free)(Sensor * sensor);

typedef struct Charge {
  
  char gpio;        // the broadcom number of the pin
  bool hot;         // whether wire should be hot or cold
  int  duration;    // used for pulsing
  
} Charge;

typedef struct Numeric {
  
  union {
    int   integer;    // the numerical value 
    float decimal;    // -------------------
  };
  
  char units[8];      // the units code
  bool is_decimal;    // representation
  
} Numeric;

typedef struct Trigger {
  
  char * id;              // target (like A01)
    
  Numeric * threshold;    // threshold for condition
  List    * charges;      // wires for actuation
  
  bool less;
  bool fired;
  bool singular;
  bool reverses;
  
} Trigger;


typedef struct Sensor {
  
  char * name;              // component name
  char * code_name;         // abbreviated name
  bool   print;             // whether sensor prints
  
  union {
    i2c_device * i2c;       // i2c communications info
    one_device * one;       // 1-wire communications info
  };
  int bus;                  // which bus is used
  
  List    * triggers;       // sensor triggers
  Hashmap * targets;        // that which can be triggered
  Hashmap * calibrations;   // calibrations for each target
  Hashmap * output_units;   // units for each trigger
  
  float auto_regressive;    
  
  sensor_free free;      // how to free sensor
  
  void * data;           // specialized sensor data
  
} Sensor;

typedef struct ProtoSensor {
  
  char * code_name;          // abbreviated name
  
  int hertz;                 // bus communication frequency in hertz
  int hertz_denominator;     // engenders fractional frequency through deferrals
  uint8 address;             // i2c address
  
  int bus;                   // which bus this is connected to
  
  List    * triggers;        // gpio triggers
  Hashmap * targets;         // that which can be triggered
  Hashmap * calibrations;   // calibrations for each target
  Hashmap * output_units;    // units for each trigger
  
  float auto_regressive;     
  
  bool print;                // whether sensor is printed to console
  bool requested;            // whether sensor is requested
  
} ProtoSensor;

typedef struct Schedule {
  
  List * i2c_devices;          // list of all i2c device pointers
  List * one_devices;          // list of all 1-wire device pointers
  
  long i2c_interval;           // scheduler spacing for the i2c protocol
  long one_interval;           // scheduler spacing for the 1-wire protocol
  
  bool i2c_active;             // whether experiment uses i2c
  bool one_active;             // whether experiment uses 1-wire
  
  int   interrupts;            // interrupts since schedule creation
  float interrupt_interval;    // time between each interrupt
  
  Thread * i2c_thread;         // i2c thread
  Thread * one_thread;         // 1-wire thread (deprioritized)
  bool term_signal;            // when set to true, schedule terminates
  
} Schedule;

Schedule * schedule;


List    * sensors;          // every active sensor on craft
Hashmap * proto_sensors;    // sensors that could be specified

int n_triggers;             // number of triggers

float to_standard_units(Numeric * dest, Numeric * source);
float convert_to(char * units, float value);
float time_passed();    // time since experiment start

void init_sensors();
void start_sensors();
void terminate_sensors();
Sensor * sensor_from_proto(ProtoSensor *);

#endif
