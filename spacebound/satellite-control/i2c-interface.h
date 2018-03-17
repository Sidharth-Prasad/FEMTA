#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE
#define HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE

#include "graphics.h"

typedef struct I2C {

  unsigned char i2c_address;
  short * registers;

  // Read Functions
  void  (* gyros)(float * axes);
  void  (* accelerometers)(float * axes);
  float (* temperature)();

} I2C;

bool initialize_i2c(module * initialent);
void printStartupConstants(char * offset);
void terminate_mpu_logging();

module * i2c_device;
Plot * mpu_gyro_plot;
Plot * mpu_acel_plot;
Plot * mpu_magn_plot;

#endif
