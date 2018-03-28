/**
 * The following file enables serial communication with the BNO-055. 
 * Base functions come from Steven Pugia's C# library, implemented 
 * using pigpio analogs to the C#-specific commands.
 * 
 */

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pigpio.h>
#include <stdbool.h>
#include <pthread.h>

#include "unified-controller.h"
#include "serial-interface.h"
#include "linked-list.h"
#include "graphics.h"
#include "timing.h"
#include "colors.h"

#define BNO055_ACCEL_DATA_X_LSB_ADDR 0x08
#define BNO055_MAG_DATA_X_LSB_ADDR   0x0E
#define BNO055_TEMP_ADDR             0X34

void read_bytes(unsigned char handle, signed char address, signed char * buffer, signed char length) {
  
  signed char command[4] = {
    0xAA,
    0x01,
    BNO055_MAG_DATA_X_LSB_ADDR & 0xFF,
    //address & 0xFF,
    length  & 0xFF,
  };

  int com_write_success = serWrite(handle, command, 4);
  

  while (serDataAvailable(handle) < 2);
  int8_t com_response[2];
  int read_success = serRead(handle, com_response, 2);

  //buffer[0] = read_success;
  //if (com_response[0] != 0xBB) return;  
  
  while (serDataAvailable(handle) < length);
  read_success = serRead(handle, buffer, length);

  buffer[0] = read_success;
}

float readSerialTempData() {

  int length = 6;
  int8_t rawData[length];
  for (int i = 0; i < length; i++) rawData[i] = 0;
  
  read_bytes(serial_device -> uart -> serial_handle, BNO055_MAG_DATA_X_LSB_ADDR, rawData, length);

  float axes[3];
  for (int i = 0; i < length; i += 2) {
    int entry = ((rawData[i + 1] << 8) | rawData[i]);
    if (entry > 32767) entry -= 65536;
    axes[i / 2] = entry / 16.0;
  }
  
  return axes[1];
}

float read_serial_magnetometers(float * axes) {
  
}

float read_serial_accelerometers(float * axes) {

}

float read_serial_gyroscopes(float * axes) {

}

bool initialize_UART(module * initialent) {
  
  initialent -> uart = malloc(sizeof(UART));
  serial_device = initialent;
  serial_device -> uart -> temperature = &readSerialTempData;

  // Reset the BNO-055
  set_voltage(serial_device -> pins + 2, 0);
  nano_sleep(10000000);
  set_voltage(serial_device -> pins + 2, 1);
  nano_sleep(650000000);
  
  serial_device -> uart -> serial_handle = serOpen("/dev/ttyAMA0", 115200, 0);
  
  if (serial_device -> uart -> serial_handle < 0) return false;
  
  // Graphics memory allocation
  bno_gyro_plot = create_plot("    BNO Gyro Axes v.s. Time    ", 3);
  bno_acel_plot = create_plot("BNO Acelerometer Axes v.s. Time", 3);
  bno_magn_plot = create_plot("BNO Magnetometer Axes v.s. Time", 3);  
  
  return true;
}

void terminate_bno_logging() {
  
  if (serial_device -> uart -> serial_handle >= 0) serClose(serial_device -> uart -> serial_handle);
}
