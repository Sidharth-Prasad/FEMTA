#ifndef HEADER_GAURD_FEMTA
#define HEADER_GAURD_FEMTA

#include <stdbool.h>
#include <time.h>

#include "linked-list.h"
#include "serial.h"

typedef unsigned char uchar;

typedef struct pin {

  char state;                    // The input output state of the pin
  char logical;                  // The logical broadcom number of the pin
  char physical;                 // The physical pin number on the board

  union {
    char voltage;                // The voltage when not under pulse width modulation
    unsigned char duty_cycle;    // The duty cycle for pulse width modulation
  };

  uchar graphic;                 // The state graphic this pin is represented with
} pin;

typedef struct I2C I2C;          // Forward declaration
typedef struct UART UART;        // Forward declaration
typedef struct module {

  char * identifier;             // The name of the module
  pin  * pins;                   // The pins bound to the module
  char n_pins;                   // The number of pins bound to the module

  I2C * i2c;
  Serial * serial;
  List * plots;                  // List of plots for graphing

  bool initialized;
  bool loaded;
  bool enabled;                  // Whether module is actually on the sat

  bool show_pins;                // Whether pins should be drawn in the config pane

} module;

module ** modules, * CPU, * MPU, * UM7, * Valve, * MPRLS, * FEMTA, * QB;

time_t start_time;

void set_voltage(pin * p, char voltage);

void set_pwm(pin * p, unsigned char duty_cycle);

#include "i2c-interface.h"       // Promise fulfilled for the compiler
#endif
