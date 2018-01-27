
#include <stdlib.h>
#include <pigpio.h>
#include <stdio.h>

#define RED   "\e[0;31m"
#define GREEN "\e[0;32m"
#define RESET "\e[0m"

typedef struct pin {

  char logical;         // The logical broadcom number of the pin
  char physical;        // The physical pin number on the board

  short state;          // The input output state of the pin
  
  union {
    short duty_cycle;   // The duty cycle for pulse width modulation
    short voltage;      // The voltage when not under pulse width modulation
  };
  
} pin;

typedef struct module {

  char * identifier;

  pin * pins;

} module;

module * BNO, * MPU, * Valve, * FEMTA;

void initialize_pin(pin initialent, int logical, int physical, short state) {
  initialent.state    = state;
  initialent.logical  = logical;
  initialent.physical = physical;

  initialent.voltage = 0;
  gpioSetMode(logical, state);
  if (state == PI_OUTPUT) gpioWrite(logical, 0);
}

void initialize_satellite() {

  if (gpioInitialise() < 0) {
    printf(RED "a critical error has occured\n" RESET);
    exit(1);
  }

  // Get space for modules
  BNO   = malloc(sizeof(module));
  MPU   = malloc(sizeof(module));
  Valve = malloc(sizeof(module));
  FEMTA = malloc(sizeof(module));

  // Set module identifiers for printing
  BNO   -> identifier = "BNO 055";
  MPU   -> identifier = "MPU 9250";
  Valve -> identifier = "Valve";
  FEMTA -> identifier = "FEMTA";

  // Get space for module pin arrays
  BNO   -> pins = malloc(3 * sizeof(pin));
  MPU   -> pins = malloc(2 * sizeof(pin));
  Valve -> pins = malloc(    sizeof(pin));
  FEMTA -> pins = malloc(4 * sizeof(pin));

  // The BNO has the UART interface
  initialize_pin(BNO -> pins[0], 14,  8, PI_INPUT);   // UART TXD
  initialize_pin(BNO -> pins[1], 15, 10, PI_INPUT);   // UART RXD
  initialize_pin(BNO -> pins[2], 23, 16, PI_OUTPUT);

  // The MPU has the I2C interface
  initialize_pin(MPU -> pins[0],  2,  3, PI_INPUT);   // I2C SDA
  initialize_pin(MPU -> pins[1],  3,  5, PI_INPUT);   // I2C SCL

  // The Valve is controlled via digital states
  initialize_pin(Valve -> pins[0], 17, 11, PI_OUTPUT);
  
  // The FEMTA is controlled via pulse width modulation
  initialize_pin(FEMTA -> pins[0], 24, 18, PI_OUTPUT);
  initialize_pin(FEMTA -> pins[1], 25, 22, PI_OUTPUT);
  initialize_pin(FEMTA -> pins[2], 27, 13, PI_OUTPUT);
  initialize_pin(FEMTA -> pins[3], 22, 15, PI_OUTPUT);

  printf("\n" GREEN "satellite initialized successfully!" RESET "\n\n");
}

void terminate_satellite() {

  // Set all output pin voltages to 0
  gpioWrite(BNO   -> pins[2].logical, 0);
  gpioWrite(Valve -> pins[0].logical, 0);
  for (int p = 0; p < 4; p++) gpioWrite(FEMTA -> pins[p].logical, 0);
  
  gpioTerminate();
}

int main() {

  initialize_satellite();
  
  terminate_satellite();
  return 0;
}
