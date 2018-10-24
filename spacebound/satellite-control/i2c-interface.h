#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE
#define HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_I2C_INTERFACE

#include "graphics.h"
#include "logger.h"

#define I2C_ACK   0x1
#define I2C_NACK  0x0
#define I2C_START 0x1
#define I2C_STOP  0x0
#define I2C_READ  0x1
#define I2C_Write 0x0

typedef unsigned char uchar;

typedef struct I2C {

  uchar i2c_address;
  uchar i2c_slave_address;
  
  short * registers;
  
  uchar frequency;            // Delays until read is performed (lower is more frequent)
  uchar delays_passed;        // Delays that have passed since last read
  
} I2C;

typedef struct module module;

bool initialize_i2c();
void printStartupConstants(char * offset);
void terminate_i2c();

long i2c_delay;    // how long between bursts of use (in nanoseconds)

Logger * mpu_logger;
Logger * mprls_logger;

#endif
