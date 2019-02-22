

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pigpio.h>

#include "clock.h"
#include "color.h"
#include "i2c.h"
#include "list.h"
#include "selector.h"
#include "types.h"


int main(int argc, char ** argv) {
  
  // start pigpio library
  if (gpioInitialise() < 0) {
    printf(RED "pigpio unable to start\n" RESET);
    exit(2);
  }
  
  if (argc > 1) {
    printf("t:ADXL 345\n");
    printf("a:x-axis\n");
    printf("a:y-axis\n");
    printf("a:z-axis\n");
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
  
  // tell threads to terminate
  schedule -> term_signal = true;
  
  // join with threads
  pthread_join(*schedule -> thread, NULL);
  
  terminate_i2c();
  
  // terminate pigpio library
  gpioTerminate();
  
  return EXIT_SUCCESS;
}
