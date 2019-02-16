

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "ds32.h"
#include "fram.h"
#include "color.h"

#define FRAM_ADDRESS 0x50

void free_fram(Sensor * fram);
bool read_fram(i2c_device * fram_i2c);


Sensor * init_fram() {
  
  Sensor * fram = malloc(sizeof(Sensor));
  
  fram -> name = "FRAM";
  fram -> free = free_fram;
  
  fram -> file = fopen("logs/fram.log", "a");
  
  fprintf(fram -> file, GRAY "\n\nFRAM\nStart time %s\n" RESET, formatted_time);
  
  fram -> i2c = create_i2c_device(fram, FRAM_ADDRESS, read_fram, 100);    // 100ms between reads
  
  return fram;
}

bool read_fram(i2c_device * fram_i2c) {
  
  uint8 read_raws[7];
  
  //i2c_read_bytes(fram_i2c, 0x00, read_raws, 7);
  
  return true;
}

void free_fram(Sensor * fram) {
  
}
