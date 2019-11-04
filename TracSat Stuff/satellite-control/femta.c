
// System libraries
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>

// Program headers
#include "femta.h"
#include "i2c-interface.h"
#include "serial.h"
#include "temperature.h"
#include "graphics.h"
#include "selector.h"
#include "controller.h"
#include "scripter.h"
#include "linked-list.h"
#include "logger.h"
#include "colors.h"
#include "error.h"

#define NUMBER_OF_MODULES 7


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
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    modules[m] = malloc(sizeof(module));
    modules[m] -> loaded = false;
    modules[m] -> initialized = false;
  }
  
  // All modules should be grouped together
  CPU   = modules[0];
  MPU   = modules[1];
  UM7   = modules[2];
  Valve = modules[3];
  MPRLS = modules[4];
  QB    = modules[5];
  FEMTA = modules[6];

  // Set module identifiers for printing
  CPU   -> identifier = "ARM 6L";
  MPU   -> identifier = "MPU 9250";
  UM7   -> identifier = "UM7";
  Valve -> identifier = "Valve";
  MPRLS -> identifier = "MPRLS";
  QB    -> identifier = "Quad Bank";
  FEMTA -> identifier = "FEMTA";

  // Set each module's number of pins
  CPU   -> n_pins = 0;
  MPU   -> n_pins = 2;
  UM7   -> n_pins = 2;
  Valve -> n_pins = 1;
  MPRLS -> n_pins = 2;
  QB    -> n_pins = 4;
  FEMTA -> n_pins = 4;

  // Let system know which are present on the sat
  CPU   -> enabled = true;
  MPU   -> enabled = true;
  UM7   -> enabled = true;
  Valve -> enabled = true;
  MPRLS -> enabled = true;
  QB    -> enabled = true;
  FEMTA -> enabled = false;

  // Let graphics know which configurations to print
  CPU   -> show_pins = false;
  MPU   -> show_pins = true;
  UM7   -> show_pins = true;
  Valve -> show_pins = true;
  MPRLS -> show_pins = true;
  QB    -> show_pins = true;
  FEMTA -> show_pins = false;
  
  // Get space for module pin arrays
  for (char m = 0; m < NUMBER_OF_MODULES; m++)
    modules[m] -> pins = malloc((modules[m] -> n_pins) * sizeof(module));

  // The MPU uses the I2C interface
  initialize_pin(&(MPU -> pins[0]),  2,  3, I2C_STATE);  // I2C SDA
  initialize_pin(&(MPU -> pins[1]),  3,  5, I2C_STATE);  // I2C SCL

  // The MPRLS attatches to the I2C interface
  initialize_pin(&(MPRLS -> pins[0]),  2,  3, I2C_STATE);  // I2C SDA
  initialize_pin(&(MPRLS -> pins[1]),  3,  5, I2C_STATE);  // I2C SCL

  // The UM7 uses the Serial UART interface
  initialize_pin(&(UM7 -> pins[0]), 14,  8, UART_STATE);   // UART TXD
  initialize_pin(&(UM7 -> pins[1]), 15, 10, UART_STATE);   // UART RXD
  
  // The Valve is controlled via digital states
  initialize_pin(&(Valve -> pins[0]), 12, 32, PI_OUTPUT);
  
  // The FEMTA is controlled via pulse width modulation
  initialize_pin(&(FEMTA -> pins[0]), 24, 18, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[1]), 25, 22, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[2]), 27, 13, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[3]), 22, 15, PI_OUTPUT);

  // The quad bank pins are: 0 = CW, 1 = CCW, and 2 is the PWM control. The circuit
  // is set up with the 0 and 1 pins acting as "on" switches; so setting 0 to HIGH
  // will turn on the CW engines, and then 2 will act as the throttle
  initialize_pin(&(QB -> pins[0]), 23, 16, PI_OUTPUT);
  initialize_pin(&(QB -> pins[1]), 18, 12, PI_OUTPUT);
  initialize_pin(&(QB -> pins[2]), 24, 18, PI_OUTPUT);
  initialize_pin(&(QB -> pins[3]), 25, 22, PI_OUTPUT);
  
  // Create the plot data for each module
  CPU   -> plots = create_list_from(1,
				    create_plot("    Temperatures v.s. Time     ", 1, 8));
  MPU   -> plots = create_list_from(3,
				    create_plot("    MPU Gyro Axes v.s. Time    ", 3, 32),
				    create_plot("MPU Acelerometer Axes v.s. Time", 3, 32),
				    create_plot("MPU Magnetometer Axes v.s. Time", 3, 32));
  UM7   -> plots = create_list_from(5,
				    create_plot("        UM7 Euler Angles       ", 3, 32),
				    create_plot("      UM7 Euler Velocities     ", 3, 32),
                                    create_plot("     UM7 Magnitometer Axes     ", 3, 32),
                                    create_plot("       UM7 Gyroscope Axes      ", 3, 32),
                                    create_plot("     UM7 Acelerometer Axes     ", 3, 32));
  Valve -> plots = NULL;
  MPRLS -> plots = create_list_from(1,
				    create_plot("    MPRLS Pressure v.s. Time   ", 1, 64));
  QB    -> plots = NULL;
  FEMTA -> plots = NULL;

  // Load plots into array of all possible owners
  all_possible_owners    = malloc(10 * sizeof(Plot *));
  all_possible_owners[0] = (Plot *) CPU   -> plots -> head                                 -> value;
  all_possible_owners[1] = (Plot *) MPU   -> plots -> head                                 -> value;
  all_possible_owners[2] = (Plot *) MPU   -> plots -> head -> next                         -> value;
  all_possible_owners[3] = (Plot *) MPU   -> plots -> head -> next -> next                 -> value;
  all_possible_owners[4] = (Plot *) MPRLS -> plots -> head                                 -> value;
  all_possible_owners[5] = (Plot *) UM7   -> plots -> head                                 -> value;
  all_possible_owners[6] = (Plot *) UM7   -> plots -> head -> next                         -> value;
  all_possible_owners[7] = (Plot *) UM7   -> plots -> head -> next -> next                 -> value;
  all_possible_owners[8] = (Plot *) UM7   -> plots -> head -> next -> next -> next         -> value;
  all_possible_owners[9] = (Plot *) UM7   -> plots -> head -> next -> next -> next -> next -> value;
  
  // Set up the interfaces
  bool i2c_success    = initialize_i2c();
  bool serial_success = initialize_serial();
  bool pid_success    = initialize_PID();       // Not used
  
  // Set each module's initialization state
  //  MPU   -> initialized = i2c_success;
  if (Valve -> enabled) Valve -> initialized = true;
  if (FEMTA -> enabled) FEMTA -> initialized = true;
  if (QB    -> enabled) QB    -> initialized = true;

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

  // Tell the sensor threads and pigpio library to terminate
  terminate_temperature_monitoring();
  terminate_i2c();
  terminate_serial();
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

  start_time = time(NULL);

  // Create the control logger
  Logger * control_logger = create_logger("./logs/control-log.txt");
  control_logger -> open(control_logger);
  fprintf(control_logger -> file,
	  YELLOW "\nRecording Control Data\nDevice\tDevice State\tMPU Measures\tSystem Time\n" RESET);
  control_logger -> close(control_logger);

  // Create message logger
  Logger * message_logger = create_logger("./logs/message-log.txt");
  message_logger -> open(message_logger);
  fprintf(message_logger -> file, PURPLE "\nStarting message log\n");
  fprintf(message_logger -> file,        "MPRLS Measures\tSystem Time\tMessage\n" RESET);
  message_logger -> close(message_logger);
  
  // Initializations
  initialize_error_handling();    // Set up the error log
  initialize_satellite();         // Set up sensors and start their threads
  print_configuration();          // Print configuration to console in case of crash
  
  initialize_graphics();          // Set up the graphical system
  initialize_scripter();          // Set up the menu system

  owner_index_list = create_list(0, true, false);   // Doublly linked list of owners

  // Temperature plot no matter what
  list_insert(owner_index_list, create_node((void *) 0));
  
  //Node * graph_owner_index_node = owner_index_list -> head;
  graph_owner_index_node = owner_index_list -> head;
  graph_owner = all_possible_owners[(int) (graph_owner_index_node -> value)];


  // State of the interface
  bool user_input = true;           // Whether we are accepting input
  
  // Allocate space for selectors
  Selector * main_menu = create_selector(NULL);
  Selector * manual    = create_selector(main_menu);
  Selector * scripts   = create_selector(main_menu);
  Selector * auto_menu = create_selector(main_menu);
  Selector * pid_menu  = create_selector(main_menu);
  Selector * graph     = create_selector(main_menu);

  // Make the menus
  add_selector_command(main_menu, 'q', "Quit"            , (lambda)       flip_bool,  (void *)    &user_input);
  add_selector_command(main_menu, 'r', "Redraw"          ,         clear_and_redraw,                     NULL);  
  add_selector_command(main_menu, 's', "Run script"      , (lambda) change_selector,  (void *)        scripts);
  add_selector_command(main_menu, 'm', "Manual control"  , (lambda) change_selector,  (void *)         manual);
  add_selector_command(main_menu, 'a', "Auto control"    , (lambda) change_selector,  (void *)      auto_menu);
  add_selector_command(main_menu, 'p', "PID control"     , (lambda) change_selector,  (void *)       pid_menu);
  add_selector_command(main_menu, 'f', "Flip views"      ,     switch_to_full_graph,                    graph);
  add_selector_command(main_menu, 'c', "Cycle graph"     , (lambda)     cycle_graph,                     NULL);
  
  add_selector_command(   manual, 'm', "Write message"   , (lambda)   write_message,  (void *) message_logger);
  add_selector_command(   manual, 'v', "Valve"           , (lambda)      flip_valve,  (void *) message_logger);
  add_selector_command(   manual, 'r', "Rotate"          , (lambda)      do_nothing,                     NULL);
  add_selector_command(   manual, 'p', "QB PWM"          , (lambda)      do_nothing,                     NULL);
  add_selector_command(   manual, 'e', "QB CCW"          , (lambda)      do_nothing,                     NULL);
  add_selector_command(   manual, 'w', "QB CW"           , (lambda)      do_nothing,                     NULL);
//add_selector_command(   manual, '0', "FEMTA 0"         , (lambda)      flip_femta,  (void *)              0);
//add_selector_command(   manual, '1', "FEMTA 1"         , (lambda)      flip_femta,  (void *)              1);
//add_selector_command(   manual, '2', "FEMTA 2"         , (lambda)      flip_femta,  (void *)              2);
//add_selector_command(   manual, '3', "FEMTA 3"         , (lambda)      flip_femta,  (void *)              3);
  
  add_selector_command(  scripts, 'i', "Test"            , (lambda)  execute_script,  (void *)       "test.x");
  add_selector_command(  scripts, 'e', "Example"         , (lambda)  execute_script,  (void *)    "example.x");
  add_selector_command(  scripts, 't', "Tuner"           , (lambda)  execute_script,  (void *)      "tuner.x");
  
  add_selector_command(auto_menu, 'x', "Ramp 0-100%"     , (lambda)         ramp_up,                     NULL);
  add_selector_command(auto_menu, 'y', "Pyramid 0-100-0%", (lambda)         pyramid,                     NULL);
//add_selector_command(auto_menu, 'z', "Configuration"   , (lambda)      do_nothing,                     NULL);
  
//add_selector_command( pid_menu, 't', "test w/ data"    , (lambda)      do_nothing,                     NULL);
  add_selector_command( pid_menu, 't', "Tune PID"        , (lambda)        PID_tune,                     NULL);
  add_selector_command( pid_menu, 'p', "Stop PID"        , (lambda)        PID_stop,                     NULL);
  add_selector_command( pid_menu, 's', "Start PID"       , (lambda)       PID_start,                     NULL);
  
  add_selector_command(    graph, 'f', "full experiment" , (lambda) change_selector,  (void *)       pid_menu);
  add_selector_command(    graph, 'i', "Increase scale"  , (lambda) change_selector,  (void *)       pid_menu);
  add_selector_command(    graph, 'd', "Decrease scale"  , (lambda) change_selector,  (void *)       pid_menu);

  
  visible_selector = main_menu;
  present_selector(visible_selector);
  
  char input;
  while (user_input) {
    input = getc(stdin);

    execute_selector(visible_selector, input);
  }
  
  
  /*  while (user_input) {
    
    input = getc(stdin);

    int mpu_reads = 0;
    
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
	
	fprintf(logger -> file, "Valve\t%d\t%d\t%d\n",
		Valve -> pins -> voltage, mpu_reads, time(NULL) - start_time);
	logger -> close(logger);
	
	break;
      case 'p':
	
	// Log the pump down message
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	
	fprintf(logger -> file, "Pump\t%d\t%d\t%d\n", 1, mpu_reads, time(NULL) - start_time);
	logger -> close(logger);
      }
    }
    
    }*/

  printf("\n");

  terminate_satellite();
  terminate_graphics();
  terminate_error_handling();
  
  control_logger -> open(control_logger);
  fprintf(control_logger -> file, YELLOW "\nTerminated gracefully at time %d seconds" RESET "\n\n", time(NULL) - start_time);
  control_logger -> close(control_logger);
  control_logger -> destroy(control_logger);

  message_logger -> open(message_logger);
  fprintf(message_logger -> file, PURPLE "\nTerminated gracefully at time %d seconds" RESET "\n\n", time(NULL) - start_time);
  message_logger -> close(message_logger);
  message_logger -> destroy(message_logger);
  
  return 0;
}
