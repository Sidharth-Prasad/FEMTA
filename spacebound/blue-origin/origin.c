

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "list.h"
#include "types.h"
#include "clock.h"
#include "i2c.h"
  
//#include <linux/i2c-dev.h>
//#include <fcntl.h>
//#include <i2c/smbus.h>


int main() {
  
  init_i2c();        // set up the i2c data structures
  init_sensors();    // set up sensor info and actions
  start_i2c();       // start reading the i2c bus
  
  real_sleep(1);

  // tell threads to terminate
  schedule -> term_signal = true;

  // join with threads
  pthread_join(*schedule -> thread, NULL);
  
  return EXIT_SUCCESS;
}
