

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "ds32.h"
#include "color.h"

#define DS32_ADDRESS 0x68

void free_ds32(Sensor * ds32);
bool read_ds32(i2c_device * ds32_i2c);

Sensor * init_ds32() {
  
  Sensor * ds32 = malloc(sizeof(Sensor));
  
  ds32 -> name = "DS3231N";
  ds32 -> free = free_ds32;
  
  ds32 -> i2c = create_i2c_device(ds32, DS32_ADDRESS, read_ds32, 1000);    // 1s between reads
  
  ds32 -> i2c -> file = fopen("logs/ds32.log", "a");
  
  setlinebuf(ds32 -> i2c -> file);    // write out every read
  
  //set_time_ds32(ds32);
  
  fprintf(ds32 -> i2c -> file, GREEN "\n\nDS3231N\n" RESET);
  
  read_ds32(ds32 -> i2c);    // read now to get human time before other sensors are created
  
  return ds32;
}

bool read_ds32(i2c_device * ds32_i2c) {
  
  uint8 read_raws[7];
  
  if (!i2c_read_bytes(ds32_i2c, 0x00, read_raws, 7)) return false;
  
  char seconds_ones = '0' + ((0b00001111 & read_raws[0]) >> 0);    // 0 tens ones
  char seconds_tens = '0' + ((0b01110000 & read_raws[0]) >> 4);    // -----------
  
  char minutes_ones = '0' + ((0b00001111 & read_raws[1]) >> 0);    // 0 tens ones
  char minutes_tens = '0' + ((0b01110000 & read_raws[1]) >> 4);    // -----------
  
  char hours_ones   = '0' + ((0b00001111 & read_raws[2]) >> 0);    // 0 am/pm 0 tens ones
  char hours_tens   = '0' + ((0b00010000 & read_raws[2]) >> 4);
  char meridian     = 'a' + ((0b00100000 & read_raws[2]) >> 5) * ('p' - 'a');
  
  char date_ones    = '0' + ((0b00001111 & read_raws[4]) >> 0);    // 00 tens ones
  char date_tens    = '0' + ((0b00110000 & read_raws[4]) >> 4);    // ------------
  
  char month_ones   = '0' + ((0b00001111 & read_raws[5]) >> 0);    // c 00 tens ones
  char month_tens   = '0' + ((0b00010000 & read_raws[5]) >> 4);    // --------------
  char century      = '0' + ((0b10000000 & read_raws[5]) >> 7);    // --------------
  
  char year_ones    = '0' + ((0b00001111 & read_raws[6]) >> 0);    // tens ones
  char year_tens    = '0' + ((0b11110000 & read_raws[6]) >> 4);    // ---------
  
  char weakday      = -1  + ((0b00000111 & read_raws[3]) >> 0);
  
  sprintf(formatted_time, "%c%c/%c%c %c%c:%c%c:%c%c %cm",
	  month_tens, month_ones, date_tens, date_ones,
	  hours_tens, hours_ones, minutes_tens, minutes_ones, seconds_tens, seconds_ones,
	  meridian);
  
  fprintf(ds32_i2c -> file, "%s\n", formatted_time);
  
  return true;
}

void set_time_ds32(Sensor * ds32) {
  
  uint8 time[7] = {
    0b00000000,  // 0 tens ones          (seconds)
    0b00010110,  // 0 tens ones          (minutes)
    0b00110001,  // 0 am/pm 0 tens ones  (hours  )
    0b00000111,  // 00000 weakday        (weekday)
    0b00010110,  // 00 tens ones         (date   )
    0b00000010,  // century 00 tens ones (month  )
    0b00011001,  // tens ones            (year   )
  };
  
  i2c_write_bytes(ds32 -> i2c, 0x00, time, 7);
}

void free_ds32(Sensor * ds32) {
  
  
  
}
