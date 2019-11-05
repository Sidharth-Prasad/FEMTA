
/** 
 * The following program contains a port of the program located in the UM7's manual.
 * 
 * Alterations have been made by Noah Franks to integrate these files into the FEMTA Cubesat
 * program. Additional code exists for specific use within FEMTA's project requirments,
 * but many of these functions can be copied as they are over to future projects involving
 * communication with the UM7 over serial UART.
 * 
 * Datasheet for binary packet structure: http://www.chrobotics.com/docs/UM7_Datasheet.pdf
 * 
 */

#include <stdbool.h> //booleans
#include <stdint.h> //int
#include <pigpio.h> //raspberry pi gpio
#include <pthread.h> //execution model to create threads for multiple workflows

#include "serial.h"
#include "femta.h"
#include "timing.h"
#include "error.h"

#include "graphics.h"
#include "colors.h"
#include "types.h"

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

pthread serial_thread;                // thread for all serial communication
bool serial_termination_signal;       // used to terminate child thread

uint8_t trans_buffer[1024];           // Stores completted UM7 packet
short   write_head;                   // trans_buffer's write head


void parse_UM7_data() {
  // Parses the trasmission data up to the write head
  
  if (write_head < 6) {
    log_error("UM7 Packet too short\n");
    return;    // Packet not long enough
  }

    
  uint16_t calculated_checksum = 's' + 'n' + 'p';
  
  for (short b = 0; b < write_head - 5; b++) {
    calculated_checksum += trans_buffer[b];
  }

  uint16_t reported_checksum = (trans_buffer[write_head - 5] << 8) | (trans_buffer[write_head - 4]);
  
  if (calculated_checksum != reported_checksum) {
    log_error("UM7 packet checksum didn't match\n");
    return;
  }


  // Get plot pointers
  Plot * um7_angl_plot = (Plot *) UM7 -> plots -> head -> value;
  Plot * um7_avel_plot = (Plot *) UM7 -> plots -> head -> next -> value;
  Plot * um7_gyro_plot = (Plot *) UM7 -> plots -> head -> next -> next -> value;
  Plot * um7_acel_plot = (Plot *) UM7 -> plots -> head -> next -> next -> next -> value;
  Plot * um7_magn_plot = (Plot *) UM7 -> plots -> head -> next -> next -> next -> next -> value;    // Not optimal
  

  // Data is correct
  uint8_t address = trans_buffer[1];
  
  switch (address) {
    
  case BATCH_PROCESSED:  

    ; // Epsilon

    uint8_t data_bytes[12][4];
    
    for (char reg = 0; reg < 12; reg++) {
      for (char byte = 0; byte < 4; byte++) {
	data_bytes[reg][byte] = trans_buffer[2 + reg * 4 + (3 - byte)];
      }
    }
    
    char b = 0;
    float gyro_x = *(float *) &data_bytes[b++];
    float gyro_y = *(float *) &data_bytes[b++];
    float gyro_z = *(float *) &data_bytes[b++];
    float gyro_t = *(float *) &data_bytes[b++];
    
    float acel_x = *(float *) &data_bytes[b++];
    float acel_y = *(float *) &data_bytes[b++];
    float acel_z = *(float *) &data_bytes[b++];
    float acel_t = *(float *) &data_bytes[b++];
    
    float magn_x = *(float *) &data_bytes[b++];
    float magn_y = *(float *) &data_bytes[b++];
    float magn_z = *(float *) &data_bytes[b++];
    float magn_t = *(float *) &data_bytes[b++];
    
    fprintf(UM7_vector_logger -> file, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
	    gyro_t, gyro_x, gyro_y, gyro_z,
	    acel_t, acel_x, acel_y, acel_z,
	    magn_t, magn_x, magn_y, magn_z);

    plot_add_value(um7_magn_plot, um7_magn_plot -> lists[0], create_node((void *)(*((int *) &magn_x))));
    plot_add_value(um7_magn_plot, um7_magn_plot -> lists[1], create_node((void *)(*((int *) &magn_y))));
    plot_add_value(um7_magn_plot, um7_magn_plot -> lists[2], create_node((void *)(*((int *) &magn_z))));

    plot_add_value(um7_gyro_plot, um7_gyro_plot -> lists[0], create_node((void *)(*((int *) &gyro_x))));
    plot_add_value(um7_gyro_plot, um7_gyro_plot -> lists[1], create_node((void *)(*((int *) &gyro_y))));
    plot_add_value(um7_gyro_plot, um7_gyro_plot -> lists[2], create_node((void *)(*((int *) &gyro_z))));

    plot_add_value(um7_acel_plot, um7_acel_plot -> lists[0], create_node((void *)(*((int *) &acel_x))));
    plot_add_value(um7_acel_plot, um7_acel_plot -> lists[1], create_node((void *)(*((int *) &acel_y))));
    plot_add_value(um7_acel_plot, um7_acel_plot -> lists[2], create_node((void *)(*((int *) &acel_z))));

    graph_plot(um7_magn_plot);
    graph_plot(um7_gyro_plot);
    graph_plot(um7_acel_plot);
    
    break;

  case BATCH_EULER:

    ; // Epsilon

    float angle_x  = ((int16_t) ((trans_buffer[2 +  0] << 8) | trans_buffer[2 +  1])) / 91.02222;
    float angle_y  = ((int16_t) ((trans_buffer[2 +  2] << 8) | trans_buffer[2 +  3])) / 91.02222;
    float angle_z  = ((int16_t) ((trans_buffer[2 +  4] << 8) | trans_buffer[2 +  5])) / 91.02222;
    
    float angle_vx = ((int16_t) ((trans_buffer[2 +  8] << 8) | trans_buffer[2 +  9])) / 16.0;
    float angle_vy = ((int16_t) ((trans_buffer[2 + 10] << 8) | trans_buffer[2 + 11])) / 16.0;
    float angle_vz = ((int16_t) ((trans_buffer[2 + 12] << 8) | trans_buffer[2 + 13])) / 16.0;

    int32_t raw_angle_t =
      (trans_buffer[2 + 16] << 24) |
      (trans_buffer[2 + 17] << 16) |
      (trans_buffer[2 + 18] <<  8) |
      (trans_buffer[2 + 19] <<  0);

    float angle_t = *(float *) &raw_angle_t;

    // Call control process, which may be the null_controller    
    serial_routine(angle_z, angle_vz, angle_t);

    // Log to file
    fprintf(UM7_euler_logger -> file, "%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
	    angle_t, angle_x, angle_y, angle_z, angle_vx, angle_vy, angle_vz);

    // Plot to terminal
    plot_add_value(um7_angl_plot, um7_angl_plot -> lists[0], create_node((void *)(*((int *) &angle_x))));
    plot_add_value(um7_angl_plot, um7_angl_plot -> lists[1], create_node((void *)(*((int *) &angle_y))));
    plot_add_value(um7_angl_plot, um7_angl_plot -> lists[2], create_node((void *)(*((int *) &angle_z))));
    
    plot_add_value(um7_avel_plot, um7_avel_plot -> lists[0], create_node((void *)(*((int *) &angle_vx))));
    plot_add_value(um7_avel_plot, um7_avel_plot -> lists[1], create_node((void *)(*((int *) &angle_vy))));
    plot_add_value(um7_avel_plot, um7_avel_plot -> lists[2], create_node((void *)(*((int *) &angle_vz))));
    
    graph_plot(um7_magn_plot);
    
    break;

  case BATCH_QUATERNION:
      
    ; // Epsilon
    
    int16_t aQuaternionRaw = (trans_buffer[2 + 0] << 8) | trans_buffer[2 + 1];
    int16_t bQuaternionRaw = (trans_buffer[2 + 2] << 8) | trans_buffer[2 + 3];
    int16_t cQuaternionRaw = (trans_buffer[2 + 4] << 8) | trans_buffer[2 + 5];
    int16_t dQuaternionRaw = (trans_buffer[2 + 6] << 8) | trans_buffer[2 + 7];
    
    float aQuaternion = aQuaternionRaw / 29789.09091;
    float bQuaternion = bQuaternionRaw / 29789.09091;
    float cQuaternion = cQuaternionRaw / 29789.09091;
    float dQuaternion = dQuaternionRaw / 29789.09091;

    int32_t tQuaternionRaw =
      (trans_buffer[2 +  8] << 24) |
      (trans_buffer[2 +  9] << 16) |
      (trans_buffer[2 + 10] <<  8) |
      (trans_buffer[2 + 11] <<  0);

    
    float tQuaternion = *(float *) &tQuaternionRaw;
    
    fprintf(UM7_quaternion_logger -> file, "%f\t%f\t%f\t%f\t%f\n",
	    tQuaternion, aQuaternion, bQuaternion, cQuaternion, dQuaternion);
    
    break;
  }
  
}


uchar packet_header_symbol = 0x0;    // used to find "s n p" header (global due to packet splitting)

void process_transmission(uint8_t * transmission, schar length) {
  // Performs two functions at the same time for performance purposes
  //
  // Raw printing
  // Prints the transmission to the serial log. This includes tiles representing each
  // byte for easy reading in various representations.
  //
  // Data parsing
  // Builds completed transmission packets using the trans_buffer global. As each is
  // completed, parse_UM7_data() is called to extract the relavent information. Because
  // the trans_buffer global exists, it's okay for a packet to be split into two reads.
  
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
      
      trans_buffer[write_head++] = transmission[index];
      
      // Look to see if packet ended
      if      (transmission[index] == 's'                               ) packet_header_symbol = 's';
      else if (transmission[index] == 'n' && packet_header_symbol == 's') packet_header_symbol = 'n';
      else if (transmission[index] == 'p' && packet_header_symbol == 'n') {
	// Packet ended, process the data
	parse_UM7_data();
	packet_header_symbol = 0x0;
	write_head = 0;
      }

      // Print integer representation
      sprintf(hex_str + offset, "    0x%02x ", transmission[index]);
      sprintf(int_str + offset, "     %03d ", transmission[index]);
      
      // Print character representation
      if      (transmission[index] == '\n') sprintf(chr_str + offset, "      \\n ");
      else if (transmission[index] == '\t') sprintf(chr_str + offset, "      \\t ");
      else if (transmission[index] == '\r') sprintf(chr_str + offset, "      \\r ");
      else if (transmission[index]  <  ' ') sprintf(chr_str + offset, "         ");
      else if (transmission[index]  >  '~') sprintf(chr_str + offset, "         ");
      else                                  sprintf(chr_str + offset, "       %c " , transmission[index]);

      // Print binary representation
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
  //fflush(serial_logger -> file);
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

  process_transmission(response, response_length);
}

void serial_write_register(uint8_t address, uint8_t D3, uint8_t D2, uint8_t D1, uint8_t D0) {
  
  uint8_t request[11];
  
  request[0] = 's';
  request[1] = 'n';
  request[2] = 'p';
  request[3] = 0b10000000;    // Has data is true
  request[4] = address;

  // Write data section
  request[5] = D3;            // highest data byte
  request[6] = D2;            // -              -
  request[7] = D1;            // -              -
  request[8] = D0;            // lowest data byte
  
  uint16_t checksum = 0;
  for (uchar b = 0; b < 9; b++) {
    checksum += request[b];
  }
  
  request[ 9] = (uint8_t) (checksum  >>  8);       // Checksum high byte
  request[10] = (uint8_t) (checksum & 0xFF);       // Checksum low byte
  
  nano_sleep(1000000000);   // Wait 1000 ms
  
  
  if (serWrite(UM7 -> serial -> handle, request, 11)) {
    log_error("Serial write failed\n");
  }
  
  nano_sleep(2000000000);   // Wait 2000 ms
  
  uint8_t response[256];
  
  schar response_length = serRead(UM7 -> serial -> handle, response, 256);

  process_transmission(response, response_length);
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
    
    process_transmission(transmission_data, communications_length);
  }

  UM7_euler_logger -> close(UM7_euler_logger);
  UM7_vector_logger -> close(UM7_vector_logger);
  UM7_quaternion_logger -> close(UM7_quaternion_logger);
  serial_logger -> close(serial_logger);
}


bool initialize_serial() {

  serial_routine = null_controller;
  
  UM7_vector_logger = create_logger("./logs/UM7-vector-log.txt");
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

void null_controller(float angle, float velocity, float time) {
  return;
}

#include <stdio.h>
int main( int argc, const char* argv[] )
{
	printf( "\nHello World\n\n" );
}
