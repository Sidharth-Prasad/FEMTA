#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_CONTROLLER
#define HEADER_GAURD_FOR_COMPILER_UNIT_UNIFIED_CONTROLLER

typedef struct pin {

  char state;                    // The input output state of the pin
  char logical;                  // The logical broadcom number of the pin
  char physical;                 // The physical pin number on the board

  union {
    char voltage;                // The voltage when not under pulse width modulation
    unsigned char duty_cycle;    // The duty cycle for pulse width modulation
  };

} pin;

typedef struct I2C I2C;          // Forward declaration
typedef struct UART UART;        // Forward declaration
typedef struct module {

  char * identifier;             // The name of the module
  pin  * pins;                   // The pins bound to the module
  char n_pins;                   // The number of pins bound to the module

  I2C * i2c;
  UART * uart;

} module;

module ** modules, * BNO, * MPU, * Valve, * FEMTA;

#include "i2c-interface.h"       // Promise fulfilled for the compiler
#include "UART-interface.h"      // Promise fulfilled for the compiler
#endif
