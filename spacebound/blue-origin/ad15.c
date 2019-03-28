

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "ad15.h"
#include "color.h"
#include "ds32.h"

#define ADXL_ADDRESS 0x53

const float adxl_bias_x =  0.0371;
const float adxl_bias_y = -0.0010;
const float adxl_bias_z = -0.0861;


void free_ad15(Sensor * ad15);
bool read_ad15(i2c_device * ad15_i2c);

Sensor * init_ad15(uchar address) {
  
  Sensor * ad15 = malloc(sizeof(Sensor));
  
  ad15 -> name = "ADS1115";
  ad15 -> free = free_ad15;
  
  ad15 -> i2c = create_i2c_device(ad15, address, read_ad15, 10);    // 10ms between reads
  
  ad15 -> i2c -> file = fopen("logs/ad15.log", "a");
  
  fprintf(ad15 -> i2c -> file, RED "\n\nAD15345\nStart time %s\nAccel x\tAccel y\tAccel z\n" RESET, formatted_time);
  
  // set up data format for acceleration measurements (page 28)
  // tell ad15 to use full resolution when measuring acceleration (bit    3)
  // tell ad15 to use full range      when measuring acceleration (bits 1,2)
  i2c_write_byte(ad15 -> i2c, 0x31, 0b00001011);    
  
  // tell ad15 to bypass it's FIFO queue (page 28)
  i2c_write_byte(ad15 -> i2c, 0x38, 0b00000000);
  
  // tell ad15 to enter measurement mode (page 26)
  i2c_write_byte(ad15 -> i2c, 0x2D, 0b00001000);    // bit 3 indicates measure mode
  
  return ad15;
}

bool read_ad15(i2c_device * ad15_i2c) {
  
  uint8 accel_raws[6];
  
  if (!i2c_read_bytes(ad15_i2c, 0x32, accel_raws, 6)) return false;
  
  int16 xAccel = (accel_raws[1] << 8) | accel_raws[0];
  int16 yAccel = (accel_raws[3] << 8) | accel_raws[2];
  int16 zAccel = (accel_raws[5] << 8) | accel_raws[4];

  /*int xAccel = (accel_raws[0] << 8) | accel_raws[1];
  int yAccel = (accel_raws[2] << 8) | accel_raws[3];
  int zAccel = (accel_raws[4] << 8) | accel_raws[5];*/

  
  //printf("%d, %d, %d\n", xAccel, yAccel, zAccel);
  //printf("Zenith %f g\n", zAccel * 3.9 / 1000.0);
  /*printf("d:%f %f %f\n",
	 xAccel * 3.9 / 1000.0,
	 yAccel * 3.9 / 1000.0,
	 zAccel * 3.9 / 1000.0);*/

  //printf("d:%f %f %f\n",
  
  fprintf(ad15_i2c -> file, "%.3f\t%.3f\t%.3f\n",
	  xAccel * 0.004 - ad15_bias_x,
	  yAccel * 0.004 - ad15_bias_y,
	  zAccel * 0.004 - ad15_bias_z);
  
  return true;
}

void free_ad15(Sensor * ad15) {
  
  
  
}
