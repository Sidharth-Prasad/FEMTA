#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE
#define HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE

typedef struct I2C {

  unsigned char i2c_address;
  short * registers;
  float (* temperature)();

} I2C;

bool initialize_i2c(module * initialent);

module * i2c_device;

#endif
