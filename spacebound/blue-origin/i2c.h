#ifndef HEADER_GAURD_I2C
#define HEADER_GAURD_I2C

#include "types.h"

typedef bool (* i2c_reader)(struct i2c_device * device);

typedef struct i2c_device {
  
  char * name;       // name of the i2c device
  
  uchar8 address;    // address on bus
  
  i2c_reader read;
  
} i2c_device;

typedef struct i2c_pattern {
  
  
  
} i2c_pattern;

typedef struct i2c_schedule {
  
  
  
} i2c_schedule;


i2c_device * create_i2c_device(char * name, uchar8 address);



#endif
