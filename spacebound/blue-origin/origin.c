

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

#include "selector.h"

int main() {

  // start pigpio library
  if (gpioInitialise() < 0) {
    printf(CONSOLE_RED "pigpio unable to start\n" CONSOLE_RESET);
    exit(2);
  }
  
  
  init_i2c();        // set up the i2c data structures
  init_sensors();    // set up sensor info and actions
  start_i2c();       // start reading the i2c bus

  Selector * selector = create_selector(NULL);
  
  add_selector_command(selector, 'q', "quit",  flip_bool, &reading_user_input);
  add_selector_command(selector, 'c', "char", output_str, (void *) "quit");
  
  reading_user_input = true;
  
  char input;
  while (reading_user_input) {
    input = getc(stdin);
    
    execute_selector(selector, input);
  }
  
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
