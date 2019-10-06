#ifndef HEADER_GAURD_I2C
#define HEADER_GAURD_I2C

#include <pthread.h>
#include <stdbool.h>

#include "../sensors/sensor.h"
#include "../structures/list.h"
#include "../types/types.h"
#include "../types/thread-types.h"

typedef struct Sensor Sensor;
typedef struct ProtoSensor ProtoSensor;
typedef struct i2c_device i2c_device;

typedef bool (* i2c_reader)(i2c_device * i2c);

typedef struct i2c_device {
  
  Sensor * sensor;
  
  FILE * log;              // log file
  
  uint8 address;           // address on bus
  
  int interval;            // time span between reads in ms
  int hertz;               // schedule frequency in hertz
  int hertz_denominator;     // engenders fractional frequency through deferrals
  int bus_deferrals;       // how many bus deferrals since last read
  int count;               // counts since last read
  
  bool reading;            // when true, scheduler knows to re-read
  
  int handle;              // pigpio handle

  int total_reads;         // total times this sensor has been read
  
  i2c_reader read;
  
} i2c_device;



void init_i2c();
void start_i2c();
void terminate_i2c();

bool i2c_write_byte (i2c_device * dev, uint8 reg, uint8 value);
bool i2c_write_bytes(i2c_device * dev, uint8 reg, uint8 * buf, char n);
bool i2c_read_bytes (i2c_device * dev, uint8 reg, uint8 * buf, char n);
bool i2c_raw_write  (i2c_device * dev,            uint8 * buf, char n);
bool i2c_raw_read   (i2c_device * dev,            uint8 * buf, char n);

uint8 i2c_read_byte(i2c_device * dev, uint8 reg);

void i2c_close(i2c_device * i2c);

i2c_device * create_i2c_device(Sensor * sensor, ProtoSensor * proto, i2c_reader reader);

#endif
