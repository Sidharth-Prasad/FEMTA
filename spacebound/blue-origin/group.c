

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "group.h"
#include "color.h"
#include "ds32.h"

void free_group(Sensor * group);
bool read_group(i2c_device * group_i2c);

Sensor * group(List * sensor_list) {
  
  Sensor * group = malloc(sizeof(Sensor));
  
  group -> name = "ADS1115";
  group -> free = free_group;

  group -> i2c = create_i2c_device(group, NULL, read_group, 10);    // 10ms between reads
  
  group -> i2c -> file = fopen("logs/group.log", "a");
  
  fprintf(group -> i2c -> file, RED "\n\nGROUP345\nStart time %s\nAccel x\tAccel y\tAccel z\n" RESET, formatted_time);
  
  
  
  return group;
}

bool read_group(i2c_device * group_i2c) {
  
  uint8 accel_raws[6];
  
  if (!i2c_read_bytes(group_i2c, 0x32, accel_raws, 6)) return false;
  
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
  
  fprintf(group_i2c -> file, "%.3f\t%.3f\t%.3f\n",
	  xAccel * 0.004 - group_bias_x,
	  yAccel * 0.004 - group_bias_y,
	  zAccel * 0.004 - group_bias_z);
  
  return true;
}

void free_group(Sensor * group) {
  
  
  
}
