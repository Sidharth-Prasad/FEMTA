
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ds18.h"
#include "ds32.h"

#include "../system/color.h"
#include "../system/gpio.h"
#include "../system/i2c.h"

void free_ds18(Sensor * sensor);
bool read_ds18(one_device * ds18_one);

Sensor * init_ds18(ProtoSensor * proto) {
  
  Sensor * ds18 = malloc(sizeof(*ds18));
  
  ds18 -> name = "DS18B20";
  ds18 -> free = free_ds18;
  
  ds18 -> print = proto -> print;
  ds18 -> targets = proto -> targets;
  ds18 -> triggers = proto -> triggers;
  
  ds18 -> one = create_one_device
    (ds18, proto, "/sys/devices/w1_bus_master1/28-0115a6756cff/w1_slave", "logs/ds18.log", proto -> hertz, read_ds18);
  
  fprintf(ds18 -> one -> log, RED "\n\nDS18B20\n Start time %s\nTemp *C\n" RESET, formatted_time);
  
  return ds18;
}

bool read_ds18(one_device * ds18_one) {
  
  Sensor * ds18 = ds18_one -> sensor;
  
  FILE * file = fopen(ds18_one -> path, "r");
  
  if (!file) {
    printf("Could not read %s for ds18: file not found\n", ds18_one -> path);
    return false;
  }
  
  char raw[128];
  
  fseek(file, 0, SEEK_END);
  int file_length = ftell(file);
  fseek(file, 0, SEEK_SET);
  fread(raw, 1, file_length, file);
  fclose(file);
  
  int temperature_code = atoi(strchr(strchr(raw, '=') + 1, '=') + 1);
  
  if (!temperature_code || temperature_code == 85000) {
    printf("Could not read %s for ds18: error code %d\n", ds18_one -> path, temperature_code);
    return false;
  }
  
  float temperature = temperature_code / 1000.0f;
  
  if (ds18 -> print)
    printf("ds18     %f\n", temperature);
  
  if (ds18 -> triggers) {
    for (iterate(ds18 -> triggers, Trigger *, trigger)) {

      if (trigger -> singular && trigger -> fired) continue;

      if ( trigger -> less && temperature > trigger -> threshold.decimal) continue;  // condition not true
      if (!trigger -> less && temperature < trigger -> threshold.decimal) continue;  // ------------------
      
      for (iterate(trigger -> charges, Charge *, charge))
	pin_set(charge -> gpio, charge -> hot);
      
      trigger -> fired = true;
    }
  }
  
  fprintf(ds18_one -> log, "%f\n", temperature);
  return true;
}

void free_ds18(Sensor * ds18) {
  
}
