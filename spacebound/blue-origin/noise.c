

#include <stdlib.h>
#include <stdio.h>

#include "color.h"
#include "ds32.h"
#include "noise.h"
#include "sensor.h"

void free_noise(Sensor * ads);
bool read_noise(i2c_device * ads_i2c);

Sensor * init_noise() {
  
  Sensor * noise = create_sensor("Noise", free_noise());
  
  return noise;
}
