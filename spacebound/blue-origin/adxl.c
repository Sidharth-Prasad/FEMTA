

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "adxl.h"

void free_adxl(Sensor * adxl);
bool read_adxl(i2c_device * adxl_i2c);


Sensor * init_adxl() {
  
  Sensor * adxl = malloc(sizeof(Sensor));
  
  adxl -> name = "ADXL345";
  adxl -> free = free_adxl;
  
  adxl -> i2c = create_i2c_device(adxl, 0x53, read_adxl, 10);    // 10ms between reads
  
  return adxl;
}

bool read_adxl(i2c_device * adxl_i2c) {
  
  printf("Pretending to read ADXL\n");
  
  return true;
}

void free_adxl(Sensor * adxl) {
  
}
