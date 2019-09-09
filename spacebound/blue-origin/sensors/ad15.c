

#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>

#include "ad15.h"
#include "ds32.h"

#include "../system/color.h"
#include "../system/gpio.h"
#include "../system/i2c.h"


// information regarding this can be found pages 18-19 of the datasheet
// more options exist in the datasheet, but this should cover most needs

#define AD15_POINTER_REG            0x00     // pointer register
#define AD15_CONFIG_REG             0x01     // configuration register

#define AD15_OS_NO_EFFECT           0b0      // do nothing
#define AD15_OS_BEGIN_CONVERSION    0b1      // begin single conversion

#define AD15_MUX_DIFFERENTIAL_AIN01 0b000    // differential between A0 and A1
#define AD15_MUX_DIFFERENTIAL_AIN23 0b011    // differential between A2 and A3
#define AD15_MUX_SINGLE_AIN0        0b100    // single channel muxed on A0
#define AD15_MUX_SINGLE_AIN1        0b101    // single channel muxed on A1
#define AD15_MUX_SINGLE_AIN2        0b110    // single channel muxed on A2
#define AD15_MUX_SINGLE_AIN3        0b111    // single channel muxed on A3

#define AD15_PGA_6144V              0b000    // analog range +/- 6.144V
#define AD15_PGA_4096V              0b001    // analog range +/- 4.096V
#define AD15_PGA_2048V              0b010    // analog range +/- 2.048V
#define AD15_PGA_1024V              0b011    // analog range +/- 1.024V
#define AD15_PGA_0512V              0b100    // analog range +/- 0.512V
#define AD15_PGA_0256V              0b101    // analog range +/- 0.256V

#define AD15_MODE_CONTINUOUS        0b0      // continually sample
#define AD15_MODE_LOW_POWER         0b1      // sample only when asked

#define AD15_RATE_8HZ               0b000    // sample at   8Hz
#define AD15_RATE_16HZ              0b001    // sample at  16Hz
#define AD15_RATE_32HZ              0b010    // sample at  32Hz
#define AD15_RATE_64HZ              0b011    // sample at  64Hz
#define AD15_RATE_128HZ             0b100    // sample at 128Hz
#define AD15_RATE_250HZ             0b101    // sample at 250Hz
#define AD15_RATE_475HZ             0b110    // sample at 475Hz
#define AD15_RATE_860HZ             0b111    // sample at 860Hz

#define AD15_COMP_TRADITIONAL       0b0      // use traditional comparator with hysteresis
#define AD15_COMP_WINDOW            0b1      // use window comparator

#define AD15_POL_ACTIVE_LOW         0b0      // set comparator to active low
#define AD15_POL_ACTIVE_HIGH        0b1      // set comparator to active high

#define AD15_LAT_DONT_LATCH         0b0      // use a non-latching comparator
#define AD15_LAT_DO_LATCH           0b1      // use a latching comparator

#define AD15_QUE_ASSERT_ONE         0b00     // assert comparator after a single conversion
#define AD15_QUE_ASSERT_TWO         0b01     // assert comparator after two conversions
#define AD15_QUE_ASSERT_FOUR        0b10     // assert comparator after four conversions
#define AD15_QUE_DISABLE            0b11     // disable comparator

void free_ad15(Sensor * ad15);
bool read_ad15(i2c_device * ad15_i2c);
void configure_ad15(Sensor * ad15);

Sensor * init_ad15(ProtoSensor * proto, char * title, List * modes, List * names) {

  Sensor * ad15 = sensor_from_proto(proto);
  
  ad15 -> name = "ADS1115";
  ad15 -> free = free_ad15;
  
  ad15 -> i2c = create_i2c_device(ad15, proto -> address, read_ad15, proto -> hertz);
  
  char file_name[32];
  
  sprintf(file_name, "logs/ad15-%x.log", proto -> address);
  
  FILE * log = fopen(file_name, "a");
  
  ad15 -> i2c -> log = log;
  
  fprintf(log, RED "\n\n");
  fprintf(log, "ADS115 - %s\n", title);
  fprintf(log, "Start time %s\n", formatted_time);
  
  for (iterate(names, char *, column_name))
    fprintf(log, "%s\t", column_name);
  
  fprintf(log, "\n" RESET);
  
  // set up the configuration  (page 19)

  AD15_Config * sensor_config = malloc(sizeof(AD15_Config));
  
  sensor_config -> COMP_QUE  = AD15_QUE_DISABLE;
  sensor_config -> COMP_LAT  = AD15_LAT_DONT_LATCH;
  sensor_config -> COMP_POL  = AD15_POL_ACTIVE_LOW;
  sensor_config -> COMP_MODE = AD15_COMP_TRADITIONAL;
  sensor_config -> DATA_RATE = AD15_RATE_860HZ;
  
  sensor_config -> MODE      = AD15_MODE_CONTINUOUS;
  sensor_config -> PGA       = AD15_PGA_6144V;
  sensor_config -> MUX       = AD15_MUX_SINGLE_AIN0;
  sensor_config -> OS        = AD15_OS_BEGIN_CONVERSION;
  
  sensor_config -> modes     = modes;
  
  ad15 -> data = sensor_config;
  
  
  // prepare for first read
  
  sensor_config -> current_mode = modes -> head;
  sensor_config -> mode_cycle = 0;
  
  uint8 first_mode = (uint8) (int) sensor_config -> current_mode -> value;
  
  sensor_config -> high_byte = (first_mode << 4) | (sensor_config -> high_byte & 0b10001111);
  
  configure_ad15(ad15);
  
  
  // console update
  
  printf("Started " GREEN "%s " RESET "at " YELLOW "%dHz " RESET "on " BLUE "0x%x " RESET,
	 ad15 -> name, proto -> hertz, proto -> address);
  
  if (proto -> print) printf("with " MAGENTA "printing\n" RESET);
  else                printf("\n");
  
  printf("logged among logs/ad15*.log\n");
  printf("An analog to digital converter\n\n");
  
  return ad15;
}

void configure_ad15(Sensor * ad15) {
  
  AD15_Config * sensor_config = ad15 -> data;
  
  uint8 config_request[3] = {
    AD15_CONFIG_REG,               // point to config register
    sensor_config -> high_byte,    // fill in the request according to the config
    sensor_config -> low_byte,     // -------------------------------------------
  };
  
  i2c_raw_write(ad15 -> i2c, config_request, 3);
}

bool read_ad15(i2c_device * ad15_i2c) {
  
  Sensor * ad15 = ad15_i2c -> sensor;
  
  AD15_Config * config = ad15 -> data;
  
  bool should_print = false;
  
  if (ad15_i2c -> hertz >= 5)
    should_print = ad15 -> print && !(ad15_i2c -> total_reads % (ad15_i2c -> hertz / 5));
  else
    should_print = ad15 -> print && !(ad15_i2c -> total_reads % (ad15_i2c -> hertz    ));
  
  ad15_i2c -> reading = true;    // this sensor does partial reads
  
  // perform the sensor read
  
  uint8 ad15_raws[2];
  
  i2c_read_bytes(ad15_i2c, 0x00, ad15_raws, 2);
  
  uint16 counts = (ad15_raws[0] << 8) | ad15_raws[1];
  
  float measure = counts;
  List * calibration = NULL;
  
  // act on potential triggers
  
  if (ad15 -> triggers) {
    for (iterate(ad15 -> triggers, Trigger *, trigger)) {
      
      if (trigger -> singular && trigger -> fired) continue;                    // singular triggers never reload
      
      int cycle = (int) hashmap_get(ad15 -> targets, trigger -> id);
      if (config -> mode_cycle != cycle) continue;                              // wrong target
      
      if ( trigger -> less && counts > trigger -> threshold -> integer) continue;  // condition not true
      if (!trigger -> less && counts < trigger -> threshold -> integer) continue;  // ------------------
      
      for (iterate(trigger -> charges, Charge *, charge)) {
	pin_set(charge -> gpio, charge -> hot);	
      }
      
      trigger -> fired = true;
    }
  }
  
  // log and print
  
  fprintf(ad15_i2c -> log, "%d\t", (int16) counts);
  
  if (config -> current_mode == config -> modes -> head)
    if (should_print)
      printf("%s ", ad15 -> code_name);
  
  if (should_print) {
    double volts = 6.114 * (double) ((int16) counts) / 32768.0;
    
    if (volts >= 0.0) printf(" ");
    printf("%.9lfv\t", volts);
  }
  
  if (config -> current_mode == config -> modes -> head -> prev) {
    // must be on last mode
    
    if (should_print) printf("\n");
    
    fprintf(ad15_i2c -> log, "\n");
    
    ad15_i2c -> reading = false;
    ad15_i2c -> total_reads++;
  }
  
  //  printf("HERE: %d\n", ad15_i2c -> reading);
  
  // change mode before leaving.
  // this is done to give the sensor time to actually flip values
  
  config -> current_mode = config -> current_mode -> next;
  config -> mode_cycle = (config -> mode_cycle + 1) % config -> modes -> size;
  
  uint8 next_mode = (uint8) (int) config -> current_mode -> value;
  
  config -> high_byte = (next_mode << 4) | (config -> high_byte & 0b10001111);
  
  configure_ad15(ad15);
  
  return true;
}

void free_ad15(Sensor * ad15) {
  // Nothing special has to happen
}
