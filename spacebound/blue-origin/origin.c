

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pigpio.h>

#include "list.h"
#include "types.h"
#include "clock.h"
#include "i2c.h"
#include "color.h"


int main() {

  // start pigpio library
  if (gpioInitialise() < 0) {
    printf(CONSOLE_RED "pigpio unable to start\n" CONSOLE_RESET);
    exit(2);
  }
  
  init_i2c();        // set up the i2c data structures
  init_sensors();    // set up sensor info and actions
  start_i2c();       // start reading the i2c bus
  
  real_sleep(60);
  
  // tell threads to terminate
  schedule -> term_signal = true;
  
  // join with threads
  pthread_join(*schedule -> thread, NULL);
  
  terminate_i2c();
  
  // terminate pigpio library
  gpioTerminate();
  
  return EXIT_SUCCESS;
}
