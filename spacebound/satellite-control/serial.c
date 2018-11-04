
/** 
 * The following program contains a port of the program located in the UM7's manual.
 * 
 * Alterations have been made by Noah Franks to integrate these files into the FEMTA Cubesat
 * program. Additional code exists for specific use within FEMTA's project requirments,
 * but many of these functions can be copied as they are over to future projects involving
 * communication with the UM7 over serial UART.
 */

#include <stdbool.h>
#include <stdint.h>
#include <pigpio.h>
#include <pthread.h>

#include "serial.h"
#include "femta.h"
#include "timing.h"
#include "error.h"

#include "graphics.h"
#include "colors.h"
#include "types.h"

// UM7 Configuration Registers
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


pthread serial_thread;                // thread for all serial communication
bool serial_termination_signal;       // used to terminate child thread


bool parse_packet(uint8_t * data, uint8_t length, Packet * packet) {
  // Parses the rx data by finding the packet (if it exists) and extracting
  // it's information. Returns NULL upon failures, which are logged to the error log.
  
  if (length < 7) {
    log_error("UM7 packet was not long enough for parsing\n");
    return false;
  }
  
  uint8_t packet_index;

  // Look for where packet header starts by finding the header starting string, "snp"
  for (uint8_t i = 0; i < length - 2; i++) {
    // Check if "snp" starts here
    
    if (data[i] == 's' && data[i + 1] == 'n' && data[i + 2] == 'p') {
      packet_index = i;
      break;
    }
  }
  
  if (packet_index == (length - 2)) {
    log_error("UM7 packet was not found\n");
    return false;
  }

  if (packet_index - length < 7) {
    log_error("UM7 packet too short for full data\n");
    return false;
  }

  
  // Packet was found, let's see if it's valid
  uint8_t type = data[length + 3];

  uint8_t data_length = 0;
  if (type >> 7 & 0x01) {
    if (type >> 6 & 0x01) data_length = 4 * ((type >> 2) & 0x0F);
    else                  data_length = 4;
  }
  else data_length = 0;

  if (length - packet_index < data_length + 5) {
    log_error("UM7 packet too short for full data\n");
    return false;
  }

  packet -> address = data[packet_index + 4];
  packet -> type = type;
  
  uint16_t checksum = 's' + 'n' + 'p' + (packet -> type) + (packet -> address);
  
  for (uint8_t i = 0; i < data_length; i++) {
    packet -> data[i] = data[packet_index + 5 + i];
    checksum += packet -> data[i];
  }

  if ((data[packet_index + 5 + data_length] << 8) | data[packet_index + 6 + data_length]) {
    log_error("UM7 packet checksum didn't match\n");
    return false;
  }

  // We made it to the end and the checksum checked out.
  // The packet has been copied, so we'll return success.
  return true;
}

void configure_serial() {

  // 
  
  
}

void print_transmission(uint8_t * transmission, schar length) {
  
  uint8_t hex_str[96];
  uint8_t chr_str[96];
  uint8_t int_str[96];
  uint8_t bin_str[96];
    
  if (length <= 0) {
    fprintf(serial_logger -> file, "Transmission Failure: %d\n", length);
  }

  fprintf(serial_logger -> file, "Transmission\nPacket Length: %d\n", length);
  
  uint8_t index  = 0;    // Data index
  
  while (index < length) {
    
    sprintf(hex_str, "Hex:   ");
    sprintf(int_str, "Int:   ");
    sprintf(chr_str, "Chr:   ");
    sprintf(bin_str, "Bin:   ");
    
    uint8_t start = 7;
    
    uint8_t offset = 7;    // Print offset
    
    while (offset < 96 - 9 && index < length) {
      sprintf(hex_str + offset, "    0x%02x ", transmission[index]);
      sprintf(int_str + offset, "     %03d ", transmission[index]);

      
      if      (transmission[index] == '\n') sprintf(chr_str + offset, "      \\n ");
      else if (transmission[index] == '\t') sprintf(chr_str + offset, "      \\t ");
      else if (transmission[index] == '\r') sprintf(chr_str + offset, "      \\r ");
      else if (transmission[index]  <  ' ') sprintf(chr_str + offset, "         ");
      else if (transmission[index]  >  '~') sprintf(chr_str + offset, "         ");
      else                                  sprintf(chr_str + offset, "       %c " , transmission[index]);
      /*if (transmission[index]) sprintf(chr_str + offset, "       %c " , transmission[index]);
	else                     sprintf(chr_str + offset, "         ");*/


      uint8_t copy = transmission[index];
      char binary[9] = {'0', '0', '0', '0', '0', '0', '0', '0', 0x00};
      char bin_index = 7;
      while (copy) {
	binary[bin_index--] = '0' + (copy & 0x01);
	copy >>= 1;
      }
      sprintf(bin_str + offset, "%s ", binary);      
      
      offset += 9;
      index++;
    }
    
    fprintf(serial_logger -> file, "%s\n", hex_str);
    fprintf(serial_logger -> file, "%s\n", int_str);
    fprintf(serial_logger -> file, "%s\n", chr_str);
    fprintf(serial_logger -> file, "%s\n", bin_str);
    fprintf(serial_logger -> file,   "\n");
  }
  
  fprintf(serial_logger -> file,   "\n");
  fflush(serial_logger -> file);
}

    
void send_serial_command(uint8_t command) {
  
  uint8_t request[7];
  
  request[0] = 's';
  request[1] = 'n';
  request[2] = 'p';
  request[3] = 0x00;       // Command Type
  request[4] = command;    // 

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

  print_transmission(response, response_length);
    
  //print(GENERAL_WINDOW, str, 1);
  
  /*Packet packet;
  
  if (parse_packet(response, response_length, &packet)) {

    switch (packet.address) {
    case GET_FW_REVISION:
      ;
      
      char string[5];
      
      string[0] = packet.data[0];
      string[1] = packet.data[1];
      string[2] = packet.data[3];
      string[3] = packet.data[4];
      string[4] = '\0';
      
      //printf("Firmware revision: %s\n", string);
      log_error(string);
      break;
      
    case 0xAD:
      
      log_error("Reset to factory\n");
      
      break;
      }
  }*/
}

void serial_write_register(uint8_t address, uint8_t D3, uint8_t D2, uint8_t D1, uint8_t D0) {
  
  uint8_t request[11];
  
  request[0] = 's';
  request[1] = 'n';
  request[2] = 'p';
  request[3] = 0b10000000;    // Has data is true
  request[4] = address;

  // Write data section
  request[5] = D0;            // lowest data byte  (yes, datasheet is backwards)
  request[6] = D1;            // -              -
  request[7] = D2;            // -              -
  request[8] = D3;            // highest data byte
  
  uint16_t checksum = 0;
  for (uchar b = 0; b < 9; b++) {
    checksum += request[b];
  }
  
  request[ 9] = (uint8_t) (checksum >> 8);         // Checksum high byte
  request[10] = (uint8_t) (checksum & 0xFF);       // Checksum low byte

  nano_sleep(1000000000);   // Wait 1000 ms

  
  if (serWrite(UM7 -> serial -> handle, request, 11)) {
    log_error("Serial write failed\n");
  }
  
  nano_sleep(2000000000);   // Wait 2000 ms
  
  uint8_t response[256];
  
  schar response_length = serRead(UM7 -> serial -> handle, response, 256);

  print_transmission(response, response_length);
}

void * serial_main() {
  
  // Set up timings
  long serial_delay =   100000000L;   // 10 Hz
  //long serial_delay = 999999999L;   // 1 Hz

  while (!serial_termination_signal) {
    
    nano_sleep(serial_delay);

    if (serDataAvailable(UM7 -> serial -> handle) <= 0) continue;
    
    uint8_t transmission_data[256];
    schar communications_length = serRead(UM7 -> serial -> handle, transmission_data, 256);
    
    print_transmission(transmission_data, communications_length);
  }

  UM7_logger -> close(UM7_logger);
  serial_logger -> close(serial_logger);
}


bool initialize_serial() {
  
  UM7_logger = create_logger("./logs/UM7-log.txt");
  UM7_logger -> open(UM7_logger);
  fprintf(UM7_logger -> file,

	  GREEN
	  "\nRecording UM7 Data\n"
	  "Gyro Time\tGyro x\tGyro y\tGyro z\t"
	  "Acel Time\tAcel x\tAcel y\tAcel z\t"
	  "Magn Time\tMagn x\tMagn y\tMagn z\n"
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
    //serial_write_register(CREG_COM_RATES4, 0b00000000, 0b00000000, 0b00000000, 10);
    serial_write_register(CREG_COM_RATES1, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No raw
    serial_write_register(CREG_COM_RATES2, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No temperature
    
    /*serial_write_register(CREG_COM_RATES5, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No telemetry
    serial_write_register(CREG_COM_RATES6, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // ------------
    serial_write_register(CREG_COM_RATES7, 0b00000000, 0b00000000, 0b00000000, 0b00000000);   // No NMEA packets */

    serial_write_register(CREG_COM_RATES4, 0b00000000, 0b00000000, 0b00000000, 0b00001010);   // 10 Hz processed
    
    serial_termination_signal = false;
    pthread_create(&serial_thread, NULL, serial_main, NULL);

    return true;
  }
  
  return false;
}

void terminate_serial() {
  serial_termination_signal = true;
}
