

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "ads.h"
#include "color.h"
#include "ds32.h"
#include "types.h"

#define ADS_POINTER 0x00
#define ADS_CONFIG  0x01


void free_ads(Sensor * ads);
bool read_ads(i2c_device * ads_i2c);

Sensor * init_ads(uint8 address, Sensor * s0, Sensor * s1, Sensor * s2, Sensor * s3) {
  
  Sensor * ads = create_sensor("ADS1115", free_ads);
  
  ads -> i2c = create_i2c_device(ads, address, read_ads, 10);    // 10ms between reads
  
  ads -> subsystem[0] = s0;
  ads -> subsystem[1] = s1;
  ads -> subsystem[2] = s2;
  ads -> subsystem[3] = s3;
  
  return ads;
}

bool read_ads(i2c_device * ads_i2c) {
  
  uint8 ads_raws[8];
  
  for (char r = 0; r < 8; r += 2)
    if (!i2c_read_bytes(ads_i2c, 0x00, ads_raws + r, 2)) return false;
  
  uint16 analog[4];
  
  for (char r = 0; r < 8; r += 2)
    analog[r / 2] = (ads_raws[r + 1] << 8 | ads_raws[r]);
  
  
  
  // apply the subsystem of sensors
  
  Sensor * subsystem[4] = ads_i2c -> sensor -> subsystem;
  
  for (char s = 0; s < 4; s++) {
    
    sub_device * sub = subsystem[s] -> sub;
    
    (sub -> read)(ads_raws + 2*s);
  }
  
  
  return true;
}

void free_ads(Sensor * ads) {
  
}
