#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <logger.h>


// UM7 Configuration Registers
//All of this are the values to setup the packets for parsing and understanding
#define CREG_COM_SETTINGS     0x00    // General communication settings
#define CREG_COM_RATES1       0x01    // Broadcast rate settings
#define CREG_COM_RATES2       0x02    // Broadcast rate settings
#define CREG_COM_RATES3       0x03    // Broadcast rate settings
#define CREG_COM_RATES4       0x04    // Broadcast rate settings
#define CREG_COM_RATES5       0x05    // Broadcast rate settings
#define CREG_COM_RATES6       0x06    // Broadcast rate settings
#define CREG_COM_RATES7       0x07    // Broadcast rate settings
#define CREG_MISC_SETTINGS    0x08    // Miscelaneous settings
#define CREG_GYRO_TRIM_X      0x0C    // Bias trim for x-axis rate gyro
#define CREG_GYRO_TRIM_Y      0x0D    // Bias trim for y-axis rate gyro
#define CREG_GYRO_TRIM_Z      0x0E    // Bias trim for z-axis rate gyro
#define CREG_MAG_CAL1_1       0x0F    // Row 1, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL1_2       0x10    // Row 1, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL1_3       0x11    // Row 1, Column 3 of magnetometer calibration matrix
#define CREG_MAG_CAL2_1       0x12    // Row 2, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL2_2       0x13    // Row 2, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL2_3       0x14    // Row 2, Column 3 of magnetometer calibration matrix
#define CREG_MAG_CAL3_1       0x15    // Row 3, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL3_2       0x16    // Row 3, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL3_3       0x17    // Row 3, Column 3 of magnetometer calibration matrix
#define CREG_MAG_BIAS_X       0x18    // Magnetometer x-axis bias
#define CREG_MAG_BIAS_Y       0x19    // Magnetometer y-axis bias
#define CREG_MAG_BIAS_Z       0x1A    // Magnetometer z-axis bias

// UM7 Data Registers
#define DREG_HEALTH           0x55    // Contains information about the health and status of the UM7
#define DREG_TEMPERATURE      0x5F    // Temperature data
#define DREG_TEMPERATURE_TIME 0x60    // Time at which temperature data was aquired
#define DREG_GYRO_PROC_X      0x61    // Processed x-axis rate gyro data
#define DREG_GYRO_PROC_Y      0x62    // Processed y-axis rate gyro data
#define DREG_GYRO_PROC_Z      0x63    // Processed z-axis rate gyro data
#define DREG_GYRO_PROC_TIME   0x64    // Time at which rate gyro data was aquired
#define DREG_ACCEL_PROC_X     0x65    // Processed x-axis accelerometer data
#define DREG_ACCEL_PROC_Y     0x66    // Processed y-axis accelerometer data
#define DREG_ACCEL_PROC_Z     0x67    // Processed z-axis accelerometer data
#define DREG_ACCEL_PROC_TIME  0x68    // Time at which accelerometer data was aquired
#define DREG_MAG_PROC_X       0x69    // Processed x-axis magnetometer data
#define DREG_MAG_PROC_Y       0x6A    // Processed y-axis magnetometer data
#define DREG_MAG_PROC_Z       0x6B    // Processed z-axis magnetometer data
#define DREG_MAG_PROC_TIME    0x6C    // Time at which magnetometer data was aquired
#define DREG_QUAT_AB          0x6D    // Quaternion elements A and B
#define DREG_QUAT_CD          0x6E    // Quaternion elements C and D
#define DREG_QUAT_TIME        0x6F    // Time at which the sensor was at the specified quaternion rotation
#define DREG_GYRO_BIAS_X      0x89    // Gyro x-axis bias estimate
#define DREG_GYRO_BIAS_Y      0x8A    // Gyro y-axis bias estimate
#define DREG_GYRO_BIAS_Z      0x8B    // Gyro z-axis bias estimate

// UM7 Commands
#define GET_FW_REVISION       0xAA    // Asks for the firmware revision
#define FLASH_COMMIT          0xAB    // Writes configuration settings to flash
#define RESET_TO_FACTORY      0xAC    // Reset all settings to factory defaults
#define ZERO_GYROS            0xAD    // Causes the rate gyro biases to be calibrated
#define SET_HOME_POSITION     0xAE    // Sets the current GPS location as the origin
#define SET_MAG_REFERENCE     0xB0    // Sets the magnetometer reference vector
#define RESET_EKF             0xB3    // Resets the EKF

// UM7 Batch addresses
#define BATCH_PROCESSED       0x61    // Processed gyro, accel, and magn batch start address
#define BATCH_QUATERNION      0x6D    // Quaternion batch start address
#define BATCH_EULER           0x70    // Euler batch batch start address

#define NUMBER_OF_MODULES 1

// Structure for holding received packet information
typedef struct UM7_packet_struct
{
    uint8_t Address;
    uint8_t PT;
    uint16_t Checksum;
    
    uint8_t data_length;
    uint8_tdata[30];
} UM7_packet;

// parses data in rx_data w/length rx_length to find packet in data
// if packet found structure packet is filled with packet data
// if not enough data parse returns 1
// if enough data but no header returns 2
// if header found but insufficient data to parse whole packet then returns 3
// if received but checksum was bad then 4
// if good packet received, fills parse_serial and returns 0


uint8_t parse_serial_data(uint8_t* rx_data, uint8_t rx_length, UM7_packet* packet)
{
    uint8_t index;

    //ensure data buffer big enough to have full packet (min is 7)
    if(rx_length < 7)
    {
        return 1;
    }

    //Try to find the snp start sequence for the packet
    for(index = 0; index < (rx_length-2);index++)
    {
        if(rx_data[index]=='s' && rx_data[index+1]=='n' && rx_data[index+2]==
        'p')
        {
            break;
        }
    }

    uint8_t packet_index = index;

    if(packet_index == (rx_length - 2))
    {
        return 2;
    }

    if((rx_length - packet_index)<7)
    {
        return 3;
    }

    uint8_t PT = rx_data[packet_index + 3];

    uint8_t packet_has_data = (PT >> 7) & 0x01;     // Check bit 7 (HAS_DATA)
    uint8_t packet_is_batch = (PT >> 6) & 0x01;      // Check bit 6 (IS_BATCH)
    uint8_t batch_length = (PT >> 2) & 0x0F; 

    uint8_t data_length = 0;

    if(packet_has_data)
    {
        if(packet_is_batch)
        {
            data_length = 4*batch_length;
        }

        else
        {
            data_length = 4;
        }
    }

    else
    {
        data_length = 0;
    }

    if((rx_length - packet_index)<(data_length + 5))
    {
        return 3;
    }
    
    packet -> Address = rx_data[packet_index + 4];
    packet -> PT = PT;

    packet->data_length = data_length;
    uint16_t computed_checksum = ‘s’ + ‘n’ + ‘p’ + packet_data->PT + packet_data->Address;
    for( index = 0; index < data_length; index++)
    {
        packet -> data[index]=rx_data[packet_index + 5 + index];

        computed_checksum += packet->data[index];
    } 

    uint16_t received_checksum = (rx_data[packet_index + 5 + data_length] << 8);

    received_checksum |= rx_data[packet_index + 6 + data_length];

    if(received_checksum != computed_checksum)
    {
        return 4;
    }

    return 0;
}

//TODO:
// How to read operation: send a data packet with "Has Data" bit cleared
// UM7 in response will send a packet back 
// with "Is Batch" and "Batch Length" set based on what was sent
//Acessing desired address
//Checking Data length
//Pulling out packet's data array
// What is the UM7 object?

/*Questions:
How do UM7 packets get received or where do i access them?
is the firmware revision part necesssary?*/

//Noah's code to send commands. could be how to request data?
void send_serial_command(uint8_t command) {
  
  uint8_t request[7];
  
  request[0] = 's';
  request[1] = 'n';
  request[2] = 'p';
  request[3] = 0x00;       // Command Type
  request[4] = command;    // Is command where I send the clear "Has Bit"?

  uint16_t checksum = 's' + 'n' + 'p' + 0x00 + command;
  
  request[5] = (uint8_t) (checksum >> 8);         // Checksum high byte
  request[6] = (uint8_t) (checksum & 0xFF);       // Checksum low byte

  nano_sleep(1000000000);   // Wait 1000 ms

  
  if (serWrite(UM7 -> serial -> handle, request, 7)) {
    log_error("Serial write failed\n");
  }
  
  nano_sleep(2000000000);   // Wait 2000 ms
  
  uint8_t response[256];
  
  schar response_length = serRead(UM7 -> serial -> handle, response, 256);

  process_transmission(response, response_length);
}

// how does he create loggers?
// logger is is own class he made.

bool initialize_serial() {

  serial_routine = null_controller;
  
  UM7_vector_logger = create_logger("./logs/UM7-vector-log.txt");
  // Where does he modify the prototype? 
  UM7_vector_logger -> open(UM7_vector_logger);
  fprintf(UM7_vector_logger -> file,

	  GREEN
	  "\nRecording vectorized UM7 attitude Data\n"
	  "Gyro Time\tGyro x\tGyro y\tGyro z\t"
	  "Acel Time\tAcel x\tAcel y\tAcel z\t"
	  "Magn Time\tMagn x\tMagn y\tMagn z\n"
	  RESET);

  UM7_euler_logger = create_logger("./logs/UM7-euler-log.txt");
  UM7_euler_logger -> open(UM7_euler_logger);
  fprintf(UM7_euler_logger -> file,

	  GREEN
	  "\nRecording UM7 euler attitude data\n"
	  "Euler Time\tAngle x\tAngle y\tAngle z\t"
	  "Angular Velocity x\tAngular Velocity y\tAngular Velocity z\n"
	  RESET);
  
  UM7_quaternion_logger = create_logger("./logs/UM7-quaternion-log.txt");
  UM7_quaternion_logger -> open(UM7_quaternion_logger);
  fprintf(UM7_quaternion_logger -> file,

	  GREEN
	  "\nRecording UM7 quaternion attitude data\n"
	  "Quat Time\tQuat A\tQuat B\tQuat C\tQuat D\n"
	  RESET);
  
  serial_logger = create_logger("./logs/serial-log.txt");
  serial_logger -> open(serial_logger);
  fprintf(serial_logger -> file, YELLOW "Serial Transmission Log\n" RESET);
  
  if (UM7 -> enabled) {
    
    UM7 -> serial = malloc(sizeof(Serial));
    
    UM7 -> serial -> handle = serOpen("/dev/ttyAMA0", 115200, 0);
    
    if (UM7 -> serial -> handle < 0) {
      log_error("Unable to open serial connection for the UM7\n");
      return false;
    }
    
    
    send_serial_command(RESET_TO_FACTORY);
    nano_sleep(1000000000);   // Wait 1s
    
    //send_serial_command(GET_FW_REVISION);
    
    // Send processed data at 10 Hz

    serial_write_register(CREG_COM_RATES1, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No raw
    serial_write_register(CREG_COM_RATES2, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No temperature
    serial_write_register(CREG_COM_RATES3, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // 10 Hz processed
    serial_write_register(CREG_COM_RATES4, 0b00000000, 0b00000000, 0b00000000, 0b00001010);   // ---------------
    serial_write_register(CREG_COM_RATES5, 0b00000000, 0b00001010, 0b00000000, 0b00000000);   // 10 Hz Euler
    serial_write_register(CREG_COM_RATES6, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No misc telemetry
    serial_write_register(CREG_COM_RATES7, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No NMEA packets
    
    
    UM7 -> initialized = true;
    
    serial_termination_signal = false;
    pthread_create(&serial_thread, NULL, serial_main, NULL);

    return true;
  }
  
  return false;
}

void terminate_serial() {
  serial_termination_signal = true;
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
  UM7   = modules[0];

  // Set module identifiers for printing
  UM7   -> identifier = "UM7";

  // Set each module's number of pins
  UM7   -> n_pins = 2;

  // Let system know which are present on the sat
  UM7   -> enabled = true;

  // Let graphics know which configurations to print
  UM7   -> show_pins = true;
  
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


