#ifndef HEADER_GAURD_FOR_COMPILER_UNIT
#define HEADER_GAURD_FOR_COMPILER_UNIT

typedef struct pin {

  char state;                    // The input output state of the pin
  char logical;                  // The logical broadcom number of the pin
  char physical;                 // The physical pin number on the board

  union {
    char voltage;                // The voltage when not under pulse width modulation
    unsigned char duty_cycle;    // The duty cycle for pulse width modulation
  };

} pin;

typedef struct I2C {

  unsigned char i2c_address;
  short * registers;

} I2C;

typedef struct UART {

  unsigned char UART_address;

} UART;

typedef struct module {

  char * identifier;             // The name of the module
  pin  * pins;                   // The pins bound to the module
  char n_pins;                   // The number of pins bound to the module

  I2C * i2c;
  UART * uart;

} module;

module ** modules, * BNO, * MPU, * Valve, * FEMTA;

#endif
