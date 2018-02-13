

// DO NOT EXECUTE THIS PROGRAM UNLESS YOU KNOW WHAT YOU ARE DOING
// THIS COULD BE HIGHLY PROBLEMATIC WITH ONLY SLIGHT MODIFICATIONS
// FOR INSTRUCTIONAL PURPOSES ONLY

// compile using 'gcc -o example struct.c -lpigpio -lrt -lpthread'

#include <pigpio.h>   // Include the pigpio GPIO libary
#include <stdlib.h>   // Include the standard library for malloc()
#include <stdio.h>    // Include the standard io library for printf()

#define PI_INPUT  0
#define PI_OUTPUT 1

typedef struct pin {

  char state;                    // The input output state of the pin
  char logical;                  // The logical broadcom number of the pin
  char physical;                 // The physical pin number on the board

  union {
    signed char voltage;         // The voltage when not under pulse width modulation
    unsigned char duty_cycle;    // The duty cycle for pulse width modulation
  };

} pin;


int main() {
  
  pin * io = malloc(sizeof(pin));   // Allocate space for the pin structure
  
  
  io -> state = PI_OUTPUT;   // Remember that this pin is set to output

  io -> logical  = 24;   // one of the FEMTA pin's broadcom identification number
  io -> physical = 18;   // one of the FEMTA pin's physical location on the board
  
  io -> voltage = -3;
  
  printf("analog voltage:   \t%d\n",    io -> voltage   );    // print the voltage
  printf("pulse width cycle:\t%u\n", io -> duty_cycle);   // print the duty cycle

  // Notice: I can use duty_cycle because it's been unionized with voltage


  
  
  gpioInitialise();   // Start up the gpio library (requires root access)
  
  gpioSetMode(io -> logical, io -> state);    // Set the pin's state
  //gpioPWM(io -> logical, io -> duty_cycle);   // Set the pulse width duty cycle to 253 / 255 Hz

  gpioTerminate();   // Terminate the gpio library
  free(io);
}

