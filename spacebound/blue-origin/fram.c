

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "fram.h"

#define FRAM_ADDRESS 0x50

void free_fram(Sensor * fram);
bool read_fram(i2c_device * fram_i2c);


Sensor * init_fram() {
  
  Sensor * fram = malloc(sizeof(Sensor));
  
  fram -> name = "FRAM";
  fram -> free = free_fram;
  
  fram -> i2c = create_i2c_device(fram, FRAM_ADDRESS, read_fram, 100);    // 100ms between reads
  
  return fram;
}

bool read_fram(i2c_device * fram_i2c) {
  
  uint8 read_raws[7];
  
  i2c_read_bytes(fram_i2c -> handle, 0x00, read_raws, 7);
  
  return true;
}

void free_fram(Sensor * fram) {
  
}
