

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pigpio.h>

#include "ds32.h"

#include "../.origin/origin.h"
#include "../math/units.h"
#include "../system/color.h"
#include "../system/gpio.h"
#include "../system/i2c.h"

void free_ds32(Sensor * ds32);
bool read_ds32(i2c_device * ds32_i2c);
bool ds32_start_square_wave(i2c_device * ds32_i2c);

void schedule_tick(int gpio, int level, uint32_t tick) {
  schedule -> interrupts++;
}

Sensor * init_ds32(ProtoSensor * proto) {
  
  Sensor * ds32 = sensor_from_proto(proto);
  
  ds32 -> name = "DS3231N";
  ds32 -> free = free_ds32;
  
  ds32 -> i2c = create_i2c_device(ds32, proto, read_ds32);
  
  ds32 -> i2c -> log = fopen("logs/ds32.log", "a");
  
  setlinebuf(ds32 -> i2c -> log);    // write out every read
  
  //set_time_ds32(ds32);
  
  fprintf(ds32 -> i2c -> log, GREEN "\n\nDS3231N\nHuman Time\tExperiment Duration [s]\tTemperature [C]\n" RESET);
  
  // Establish time experiment information
  ds32_start_square_wave(ds32 -> i2c);
  gpioSetISRFunc(20, RISING_EDGE, 0, schedule_tick);    // start counting interrupts
  read_ds32(ds32 -> i2c);                               // get human time before other sensors are created
  
  List * time_calibration = hashmap_get(ds32 -> calibrations, "Time");
  schedule -> interrupt_interval = compute_curve(1.0f, time_calibration);
  
  
  // Print a nice message to the user
  printf("Started " GREEN "%s " RESET "at " YELLOW "%dHz " RESET "on " BLUE "0x%x " RESET,
	 ds32 -> name, proto -> hertz, proto -> address);
  
  if (proto -> print) printf("with " MAGENTA "printing\n" RESET);
  else                printf("\n");
  
  printf("logged in logs/ds32.log\n");
  printf("A real time clock\n\n");
  
  return ds32;
}

bool ds32_start_square_wave(i2c_device * ds32_i2c) {
  // asks the ds32 to start emitting square waves at a rate of 1.024 kHz.
  
  /* Bits, from left to right, brackets going in binary order
   * 
   *  7: Let oscillator use battery        {no, yes} (Crazy to not say 'yes')
   *  6: Enable square wave                {no, yes} (We use this for interrupts)
   *  5: Immediately adjust to temperature {no, yes} (Happens every 64s anyways, 2ms delay if yes)
   * 34: Frequency for square wave         {1Hz, 1.024kHz, 4.096kHz, 8.192kHz} (01 yields sub-ms)
   *  2: What to use the SQR pn for        {square waves, alarms}
   *  1: Enable Alarm 2?                   {no, yes}
   *  0: Enable Alarm 1?                   {no, yes}
   * 
   */
  
  if (!i2c_write_byte(ds32_i2c, 0x0E, 0b01101000)) {
    printf(RED "Could not enable ds32 square waves!\n" RESET);
    return false;
  }
  
  uint8 reg = i2c_read_byte(ds32_i2c, 0x0E);
  
  printf("DEBUG: %u\n", reg);
  
  return true;
}

bool read_ds32(i2c_device * ds32_i2c) {
  
  Sensor * ds32 = ds32_i2c -> sensor;
  
  uint8 read_raws[9];
  
  if (!i2c_read_bytes(ds32_i2c, 0x00, read_raws + 0, 7)) return false;
  if (!i2c_read_bytes(ds32_i2c, 0x11, read_raws + 7, 2)) return false;
  
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
  
  float temperature = 1.0f * read_raws[7] + (read_raws[8] >> 6) * 0.25f;
  
  
  experiment_duration = (float) schedule -> interrupts;
  
  char * curve = hashmap_get(ds32 -> output_units, "Time");
  List * calibration = hashmap_get(ds32 -> calibrations, "Time");
  
  if (calibration) {
    experiment_duration = compute_curve(experiment_duration, calibration);
  }
  
  
  if (ds32 -> print)
    printf("%s     %s  %.4fs %.2fC\n", ds32 -> code_name, formatted_time, experiment_duration, temperature);
  
  fprintf(ds32_i2c -> log, "%s\t%.4f\t%.2f\n", formatted_time, experiment_duration, temperature);
  
  if (ds32 -> triggers) {
    for (iterate(ds32 -> triggers, Trigger *, trigger)) {
      
      if (trigger -> singular && trigger -> fired) continue;
      
      Numeric * requested_threshold = trigger -> threshold;
      Numeric threshold;
      
      to_standard_units(&threshold, requested_threshold);
      
      if ( trigger -> less && experiment_duration > threshold.decimal) continue;  // condition not true
      if (!trigger -> less && experiment_duration < threshold.decimal) continue;  // ------------------
      
      for (iterate(trigger -> charges, Charge *, charge))
	pin_set(charge -> gpio, charge -> hot);
      
      trigger -> fired = true;
    }
  }
  
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
  // Nothing special has to happpen
}
