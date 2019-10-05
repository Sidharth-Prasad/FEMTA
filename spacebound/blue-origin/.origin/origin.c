

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>

#include "origin.h"

#include "../sensors/sensor.h"
#include "../system/clock.h"
#include "../system/error.h"
#include "../system/color.h"
#include "../system/gpio.h"
#include "../system/i2c.h"
#include "../structures/list.h"
#include "../structures/selector.h"
#include "../types/types.h"
#include "../parser/y.tab.h"

bool console_error_messages = true;

FILE * yyin;
void print_config();
void parse_args(int, char **);

int main(int argc, char ** argv) {
  
  // start pigpio library
  if (gpioInitialise() < 0)
    exit_printing("pigpio unable to start\n", ERROR_OPERATING_SYSTEM);

  init_color();      // init colorized printing to the console
  init_pins();       // set up gpio data structure
  
  schedule = calloc(1, sizeof(*schedule));
  
  init_one();        // set up the 1-wire data structures
  init_i2c();        // set up the i2c data structures
  init_sensors();    // set up sensor info and actions
  
  parse_args(argc, argv);
  print_config();

  //  gpioTerminate(); exit(0);
  
  start_sensors();
  start_one();       // start reading the 1-wire bus
  start_i2c();       // start reading the i2c bus
  
  Selector * selector = create_selector(NULL);
  
  add_selector_command(selector, 'q', NULL,    flip_bool,     &reading_user_input);
  add_selector_command(selector, 'e', NULL,    flip_bool, &console_error_messages);
  add_selector_command(selector, 'm', NULL,   output_str,                    NULL);
  add_selector_command(selector, '+', NULL,  pin_set_hot,                    NULL);
  add_selector_command(selector, '-', NULL, pin_set_cold,                    NULL);
  
  reading_user_input = true;
  
  char input[32];
  while (reading_user_input) {
    fgets(input, 32, stdin);
    
    execute_selector(selector, input[0], input);
  }
  
  // tell threads to terminate
  schedule -> term_signal = true;
  
  printf("DEBUG: Active - %d %d\n", schedule -> i2c_active, schedule -> one_active);
  
  // join with threads
  if (schedule -> i2c_active) pthread_join(*schedule -> i2c_thread, NULL);
  if (schedule -> one_active) pthread_join(*schedule -> one_thread, NULL);
  
  terminate_sensors();    // close and destroy all sensor-related structures
  terminate_i2c();        // destroy i2c bus structures
  terminate_one();        // destroy 1-wire bus structures
  
  free(schedule);
  
  // terminate pigpio library
  gpioTerminate();
  terminate_color();
  
  return EXIT_SUCCESS;
}


void parse_args(int argc, char ** argv) {

  if (argc == 1) {
    // default: run default.e
    
    yyin = fopen("./experiments/default.e", "r");
    
    if (!yyin) {
      printf(RED "Experiment file does not exist\n" RESET);
      exit(1);
    }
    
    yyparse();  
    return;
  }
  
  if (!strncmp(argv[1], "file", 4)) {
    /* parse a file for specifications */
    
    char * filename = argv[1] + 5;
    
    printf("%s\n", filename);
    
    yyin = fopen(filename, "r");
    
    if (!yyin) {
      printf(RED "Experiment file %s does not exist\n" RESET, filename);
      exit(1);
    }
    
    yyparse();
    return;
  }
  
  for (int i = 1; i < argc; i++) {
    /* read command line arguments rather than file */
    
    char code_name[32];
    code_name[0] = '\0';    // need to protect buffer from previous iteration
    
    int  hertz = 0;
    bool print = false;
    
    sscanf(argv[i], "%[^*,],%d", code_name, &hertz);      
    
    if (!code_name[0]) {
      sscanf(argv[i], "*%[^,],%d", code_name, &hertz);
      print = true;
    }
    
    ProtoSensor * proto = hashmap_get(proto_sensors, code_name);
    
    //printf("%s at %d\n", code_name, hertz);
    
    if (!proto) {
      printf(RED "%s is not a sensor\n" RESET, code_name);
      exit(1);
    }
    
    proto -> requested = true;
    proto -> print     = print;
    
    if (hertz) proto -> hertz = hertz;
  }
}
