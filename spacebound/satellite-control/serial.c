
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

#include "serial.h"
#include "error.h"

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

void initialize_serial() {

  UM7 -> handle = serOpen("/dev/ttyAMA0", 115200, 0);

  send_serial_command(0xAA);
}

void send_serial_command(uint8_t command) {
  
  uint8_t request[24];
  
  request[0] = 's';
  request[1] = 'n';
  request[2] = 'p';
  request[3] = 0x00;       // Type
  request[4] = command;    // 
  request[5] = 0x01;       // Checksum high byte
  request[6] = 0xFB;       // Checksum low byte

  serWrite(UM7 -> handle, request, 7);

  uint8_t response[256];
  
  schar response_length = serRead(UM7 -> handle, response, 256);

  Packet packet;
  
  if (parse_packet(response, response_length, &packet) {

      if (packet.address == 0xAA) {

        char string[5];

        string[0] = packet.data[0];
        string[1] = packet.data[1];
        string[2] = packet.data[3];
        string[3] = packet.data[4];
        string[4] = '\0';

        printf("Firmware revision: %s\n", string);
    }

  
}
