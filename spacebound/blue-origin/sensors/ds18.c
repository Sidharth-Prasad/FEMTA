
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pigpio.h>

#include "ds18.h"
#include "ds32.h"

#include "../structures/selector.h"
#include "../system/color.h"
#include "../system/clock.h"
#include "../system/gpio.h"
#include "../system/i2c.h"


void free_ds18(Sensor * sensor);
bool read_ds18(one_device * ds18_one);

Sensor * init_ds18(ProtoSensor * proto) {
  
  Sensor * ds18 = sensor_from_proto(proto);
  
  ds18 -> name = "DS18B20";
  ds18 -> free = free_ds18;
  
  ds18 -> print    = proto -> print;
  ds18 -> targets  = proto -> targets;
  ds18 -> triggers = proto -> triggers;
  
  ds18 -> one = create_one_device
    //(ds18, proto, "/sys/bus/w1/devices/28-0115a6756cff/w1_slave", "logs/ds18.log", read_ds18);
    //(ds18, proto, "/sys/bus/w1/devices/28-000008e222e7/w1_slave", "logs/ds18.log", read_ds18);
    //(ds18, proto, "/sys/bus/w1/devices/28-000008e3f48b/w1_slave", "logs/ds18.log", read_ds18);
    (ds18, proto, "/sys/bus/w1/devices/28-0315a66ea4ff/w1_slave", "logs/ds18.log", read_ds18);

  fprintf(ds18 -> one -> log, RED "\n\nDS18B20\n Start time %s\nTemp *C\n" RESET, formatted_time);
  
  return ds18;
}

bool read_ds18(one_device * ds18_one) {
  
  Sensor * ds18 = ds18_one -> sensor;
  
  FILE * file = fopen(ds18_one -> path, "r");
  
  if (!file) {
    printf(RED "Could not read %s for ds18: %s\n" RESET, ds18_one -> path, strerror(errno));
    
    /*gpioSetMode(4, PI_OUTPUT);
    pin_set(4, 0); real_nano_sleep(3000000000);
    pin_set(4, 1); real_nano_sleep(5000000000);
    gpioSetMode(4, PI_ALT0);
    real_nano_sleep(8000000000);*/
    
    /*reading_user_input = false;
    schedule -> term_signal = true;    // completely bail out of the mission
    real_sleep(2);*/
    //kill(getppid(), SIGKILL);
    //exit(3);
    
    return false;
  }
  
  char raw[128];
  
  /*(fseek(file, 0, SEEK_END);
  int file_length = ftell(file);
  fseek(file, 0, SEEK_SET);
  fread(raw, 1, file_length, file);*/
  int len = fread(raw, 1, 127, file);
  raw[len] = '\0';
  fclose(file);
  
  int temperature_code = atoi(strchr(strchr(raw, '=') + 1, '=') + 1);
  
  if (!temperature_code || temperature_code == 85000) {
    printf(RED "Could not read %s for ds18: error code %d\n" RESET, ds18_one -> path, temperature_code);
    return false;
  }
  
  float temperature = temperature_code / 1000.0f;
  
  if (ds18 -> print)
    printf("ds18      %.5fs    %.5fC\n", time_passed(), temperature);
  
  if (ds18 -> triggers) {
    for (iterate(ds18 -> triggers, Trigger *, trigger)) {

      if (trigger -> singular && trigger -> fired) continue;

      if ( trigger -> less && temperature > trigger -> threshold -> decimal) continue;  // condition not true
      if (!trigger -> less && temperature < trigger -> threshold -> decimal) continue;  // ------------------
      
      for (iterate(trigger -> charges, Charge *, charge))
	pin_set(charge -> gpio, charge -> hot);
      
      trigger -> fired = true;
    }
  }
  
  fprintf(ds18_one -> log, "%.5fs\t%.5f\n", time_passed(), temperature);
  return true;
}

void free_ds18(Sensor * ds18) {
  // Nothing special has to happen
}
