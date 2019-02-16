

#include <stdlib.h>
#include <stdio.h>

#include "i2c.h"
#include "ds31.h"

#define DS31_ADDRESS 0x68

void free_ds31(Sensor * ds31);
bool read_ds31(i2c_device * ds31_i2c);


Sensor * init_ds31() {
  
  Sensor * ds31 = malloc(sizeof(Sensor));
  
  ds31 -> name = "DS3132N";
  ds31 -> free = free_ds32;
  
  ds31 -> i2c = create_i2c_device(ds31, DS31_ADDRESS, read_ds31, 100);    // 100ms between reads
  
  return ds31;
}

bool read_ds31(i2c_device * ds31_i2c) {
  
  uint8 read_raws[7];
  
  i2c_read_bytes(ds31_i2c -> handle, 0x00, read_raws, 7);
  
  char seconds_ones = '0' + (0b00001111 & read_raws[0]);    // tens ones
  char seconds_tens = '0' + (0b01110000 & read_raws[0]);    // ---------
  
  char minutes_ones = '0' + (0b00001111 & read_raws[1]);    // 0 tens ones
  char minutes_tens = '0' + (0b01110000 & read_raws[1]);    // -----------
  
  char hours_ones   = '0' + (0b00001111 & read_raws[2]);    // 0 am/pm 0 tens ones
  char hours_tens   = '0' + (0b00010000 & read_raws[2]);
  char meridian     = 'a' + (0b01000000 & read_raws[2]) * ('p' - 'a');
  
  char date_ones    = '0' + (0b00001111 & read_raws[4]);    // 00 tens ones
  char date_tens    = '0' + (0b00110000 & read_raws[4]);    // ------------
  
  char month_ones   = '0' + (0b00001111 & read_raws[5]);    // c 00 tens ones
  char month_tens   = '0' + (0b00010000 & read_raws[5]);    // --------------
  char century      = '0' + (0b10000000 & read_raws[5]);    // --------------
  
  char year_ones    = '0' + (0b00001111 & read_raws[6]);    // tens ones
  char year_tens    = '0' + (0b11110000 & read_raws[6]);    // ---------
  
  week_day weakday  = -1  + (0b00000111 & read_raws[3]);
  
  printf("%c%c %c%c:%c%c:%c%c %cm\n",
         month_tens, month_tens,
         hours_tens, hours_ones,
         minutes_tens, minutes_ones,
         seconds_tens, seconds_ones,
         );
  
  return true;
}

void free_ds31(Sensor * ds31) {
  
  
  
}
