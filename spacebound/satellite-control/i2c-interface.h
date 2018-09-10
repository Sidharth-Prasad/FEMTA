#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE
#define HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE

#include "graphics.h"
#include "logger.h"

typedef struct I2C {

  unsigned char i2c_address;
  unsigned char i2c_slave_address;
  
  short * registers;

  // Read Functions
  void  (* gyros)(float * axes);
  void  (* accelerometers)(float * axes);
  void  (* magnetometers)(float * axes);
  float (* temperature)();

} I2C;

typedef struct module module;

bool initialize_i2c(module * initialent);
void printStartupConstants(char * offset);
void terminate_mpu_logging();

module * i2c_device;
Plot * mpu_gyro_plot;
Plot * mpu_acel_plot;
Plot * mpu_magn_plot;

Logger * mpu_logger;

#endif
