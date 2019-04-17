

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pigpio.h>

#include "../system/clock.h"
#include "../system/color.h"
#include "../system/i2c.h"
#include "../structures/list.h"
#include "../structures/selector.h"
#include "../types/types.h"


void parse_args(int argc, char ** argv) {

  if (argc == 1) {
    // default, use all sensors
    
    for (iterate(proto_sensors -> all, ProtoSensor *, proto))
      proto -> requested = true;
    
    return;
  }
  
  
  for (int i = 1; i < argc; i++) {
    
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



int main(int argc, char ** argv) {
  
  // start pigpio library
  if (gpioInitialise() < 0) {
    printf(RED "pigpio unable to start\n" RESET);
    exit(2);
  }
  
  init_i2c();        // set up the i2c data structures
  init_sensors();    // set up sensor info and actions
  
  parse_args(argc, argv);
  
  start_sensors();
  start_i2c();       // start reading the i2c bus
  
  //exit(0);
  
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
