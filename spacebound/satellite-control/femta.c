
// System libraries
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>

// Program headers, in compilation order
#include "femta.h"
#include "i2c-interface.h"
#include "temperature-monitoring.h"
#include "graphics.h"
#include "logger.h"
#include "colors.h"

#define NUMBER_OF_MODULES 3

#define I2C_STATE 2
#define UART_STATE 3


void initialize_pin(pin * initialent, char logical, char physical, short state) {
  initialent -> state    = state;
  initialent -> logical  = logical;
  initialent -> physical = physical;

  initialent -> voltage = 0;
  if (state == PI_OUTPUT || state == PI_INPUT) gpioSetMode(logical, state);
  if (state == PI_OUTPUT) gpioWrite(logical, 0);
}

void initialize_satellite() {

  // Exit if gpio's are unavailable for some reason
  if (gpioInitialise() < 0) {
    printf(RED "a critical error has occured\n" RESET);
    exit(1);
  }
  
  // Get space for modules
  modules = malloc(NUMBER_OF_MODULES * sizeof(module *));
  for (char m = 0; m < NUMBER_OF_MODULES; m++) modules[m] = malloc(sizeof(module));
  for (char m = 0; m < NUMBER_OF_MODULES; m++) modules[m] -> loaded = false;
  
  // All modules should be grouped together
  // BNO   = modules[0];
  MPU   = modules[0];
  Valve = modules[1];
  FEMTA = modules[2];

  // Set module identifiers for printing
  // BNO   -> identifier = "BNO 055";
  MPU   -> identifier = "MPU 9250";
  Valve -> identifier = "Valve";
  FEMTA -> identifier = "FEMTA";

  // Set each module's number of pins
  // BNO   -> n_pins = 3;
  MPU   -> n_pins = 2;
  Valve -> n_pins = 1;
  FEMTA -> n_pins = 4;

  // Get space for module pin arrays
  for (char m = 0; m < NUMBER_OF_MODULES; m++)
    modules[m] -> pins = malloc((modules[m] -> n_pins) * sizeof(module));

  // The BNO has the UART interface
  //initialize_pin(&(BNO -> pins[0]), 14,  8, UART_STATE);   // UART TXD
  //initialize_pin(&(BNO -> pins[1]), 15, 10, UART_STATE);   // UART RXD
  //initialize_pin(&(BNO -> pins[2]), 23, 16, PI_OUTPUT);

  // The MPU has the I2C interface
  initialize_pin(&(MPU -> pins[0]),  2,  3, I2C_STATE);  // I2C SDA
  initialize_pin(&(MPU -> pins[1]),  3,  5, I2C_STATE);  // I2C SCL
  
  // The Valve is controlled via digital states
  initialize_pin(&(Valve -> pins[0]), 17, 11, PI_OUTPUT);
  
  // The FEMTA is controlled via pulse width modulation
  initialize_pin(&(FEMTA -> pins[0]), 24, 18, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[1]), 25, 22, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[2]), 27, 13, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[3]), 22, 15, PI_OUTPUT);

  // Set up the interfaces
  bool i2c_success    = initialize_i2c(MPU);
  //bool serial_success = initialize_UART(BNO);

  // Set each module's initialization state
  //BNO   -> initialized = true; //serial_success
  MPU   -> initialized = i2c_success;
  Valve -> initialized = true;
  FEMTA -> initialized = true;

  bool thermal_success = initialize_temperature_monitoring();
  
  // print information to the user
  printf(GREY "\nInitializing satellite\n\n" RESET);
  if (thermal_success) printf(GREEN "\tCPU\tSUCCESS\tSPAWNED\n" RESET);
  else printf(RED "\tI2C\tFAILURE\t\tUnable to read/log CPU temperature data\n" RESET);

  if (i2c_success) {
    printf(GREEN "\tI2C\tSUCCESS\tSPAWNED\n" RESET);
    printStartupConstants("\t\t");
  }
  else printf(RED "\tI2C\tFAILURE\t\tError: %d\n" RESET, i2cReadByteData(MPU -> i2c -> i2c_address, 0));

  // Serial_success is a highly falible indicator. It's a long story.
  /*if (BNO -> initialized) printf(GREEN "\tBNO\tSUCCESS\tSPAWNED\n" RESET);
    else                    printf(RED   "\tBNO\tOFFLINE\t" RESET);*/
  
  printf("\n");
  if (!(i2c_success && thermal_success)) {
    printf( RED "\nSatellite failed to initialize" RESET "\n\n");
    return;
  }
  printf(GREEN "\nSatellite initialized successfully!" RESET "\n\n");
  print(0, "Satellite initialized successfully!", 0);
}

void print_configuration() {

  // prints out a table indicating the allocation map
  printf(GREY "\tlogical   physical   state\n" RESET);
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    printf(GREY "%s\n" RESET, modules[m] -> identifier);
    for (char p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].logical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].logical);
      if (modules[m] -> pins[p].physical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].physical);

      // print out the human-readable state
      printf("         ");
      if (modules[m] -> initialized == false) printf(RED  );
      else                                    printf(GREEN);
      if      (modules[m] -> pins[p].state == PI_INPUT)   printf(RESET "Input" );
      else if (modules[m] -> pins[p].state == PI_OUTPUT)  printf(RESET "Output");
      else if (modules[m] -> pins[p].state == I2C_STATE)  printf(      "I2C"   );
      else if (modules[m] -> pins[p].state == UART_STATE) printf(      "UART"  );
      printf(RESET "\n");
    }
    printf("\n");
  }
}

void terminate_satellite() {

  // Set all output pins to 0 before exit
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    for (char p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].state == PI_OUTPUT) gpioWrite(modules[m] -> pins[p].logical, 0);
    }
  }
  
  terminate_temperature_monitoring();
  terminate_mpu_logging();
  //terminate_bno_logging();
  gpioTerminate();
}

void check_if_writeable(pin * p) {

  // scream bloody hell if this pin is read-only
  if (p -> state == PI_INPUT) {
    printf(RED "read-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

void check_if_readable(pin * p) {

  // scream bloody hell if this pin is write-only
  if (p -> state == PI_OUTPUT) {
    printf(RED "write-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

char read_voltage(pin * p) {

  check_if_readable(p);
  p -> voltage = gpioRead(p -> logical);
  return p -> voltage;
}

void set_voltage(pin * p, char voltage) {

  check_if_writeable(p);
  p -> voltage = voltage;
  gpioWrite(p -> logical, (p -> voltage > 0));
}

void set_pwm(pin * p, unsigned char duty_cycle) {

  check_if_writeable(p);
  p -> duty_cycle = duty_cycle;
  gpioPWM(p -> logical, p -> duty_cycle);
}

int main() {

  time_t start_time = time(NULL);

  // Create the control logger
  Logger * logger = create_logger("./logs/control-log.txt");
  logger -> open(logger);
  fprintf(logger -> file,
	  YELLOW "\nRecording Control Data\nDevice\tDevice State\tMPU Measures\tSystem Time\n" RESET);
  logger -> close(logger);
  
  initialize_satellite();
  print_configuration();
  
  initialize_graphics();
  
  Plot * all_possible_owners[4] = {
    temperature_plot,
    mpu_gyro_plot,
    mpu_acel_plot,
    mpu_magn_plot,
    /*bno_gyro_plot,
    bno_acel_plot,
    bno_lina_plot,
    bno_magn_plot,*/
  };

  List * owner_index_list = create_list(8);

  // Temperature plot no matter what
  list_insert(owner_index_list, create_inode(0));
  
  Node * graph_owner_index_node = owner_index_list -> head;
  graph_owner = all_possible_owners[graph_owner_index_node -> ivalue];
  
  char input;
  bool user_input = true;
  bool manual_mode = false;
  while (user_input) {
    
    input = getc(stdin);

    int mpu_reads = 0;
    //int bno_reads = 0;	
    
    if (manual_mode) {
      switch (input) {
      case '0':
      case '1':
      case '2':
      case '3':
	
	; // Epsilon
	
	char number = input - '0';    // The actual number pressed 
	
	// Flip pwm from one extrema to another
	if ((FEMTA -> pins + number) -> duty_cycle) set_pwm(FEMTA -> pins + number, 0);
	else                                        set_pwm(FEMTA -> pins + number, 255);

	// Show state change to user
	update_state_graphic(18 + number, ((FEMTA -> pins + number) -> duty_cycle > 0));

	// Log this manual command
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	//if (bno_logger) bno_reads = bno_logger -> values_read;
	fprintf(logger -> file, "FEMTA %d\t%d\t%d\t%d\n",
		number, (FEMTA -> pins + number) -> duty_cycle, mpu_reads, time(NULL) - start_time);
	logger -> close(logger);

	break;
      case 'v':
	
	// Flip valve voltage
	Valve -> pins -> voltage = !Valve -> pins -> voltage;
	set_voltage(Valve -> pins, Valve -> pins -> voltage);
	update_state_graphic(15, Valve -> pins -> voltage);

	// Log this manual command
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	//if (bno_logger) bno_reads = bno_logger -> values_read;
	fprintf(logger -> file, "Valve\t%d\t%d\t%d\n",
		Valve -> pins -> voltage, mpu_reads, time(NULL) - start_time);
	logger -> close(logger);
	
	break;
      case 'p':
	
	// Log the pump down message
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	//if (bno_logger) bno_reads = bno_logger -> values_read;
	fprintf(logger -> file, "Pump\t%d\t%d\t%d\n", 1, mpu_reads, time(NULL) - start_time);
	logger -> close(logger);
      }
    }
    
    switch (input) {
      
    case 'c':

      // Add BNO plots
      /*if (BNO -> initialized && !BNO -> loaded) {
	for (char p = 4; p <= 7; p++) list_insert(owner_index_list, create_inode(p));
	BNO -> loaded = true;
	}*/

      // Add MPU plots
      if (MPU -> initialized && !MPU -> loaded) {
	for (char p = 1; p <= 3; p++) list_insert(owner_index_list, create_inode(p));
	MPU -> loaded = true;
      }
      
      graph_owner_index_node = graph_owner_index_node -> next;
      graph_owner = all_possible_owners[graph_owner_index_node -> ivalue];
      break;

    case 'm':
      erase_print_window(1);
      print(1, "b: back", 0);
      print(1, "v: valve", 0);
      print(1, "0: FEMTA 0", 0);
      print(1, "1: FEMTA 1", 0);
      print(1, "2: FEMTA 2", 0);
      print(1, "3: FEMTA 3", 0);
      print(1, "p: Pump down", 0);
      manual_mode = true;
      break;

    case 'b':
      if (manual_mode) manual_mode = false;
      erase_print_window(1);
      print(1, "c: cycle graphs", 0);
      print(1, "m: manual control", 0);
      print(1, "q: quit", 0);
      break;
      
    case 'q':
      user_input = false;
      break;
    }
  }

  printf("\n");

  terminate_satellite();
  terminate_graphics();
  
  logger -> open(logger);
  fprintf(logger -> file, YELLOW "\nTerminated gracefully at time %d seconds" RESET "\n\n", time(NULL) - start_time);
  logger -> close(logger);
  logger -> destroy(logger);
  
  return 0;
}
