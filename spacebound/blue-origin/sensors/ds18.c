
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "ds18.h"
#include "ds32.h"

#include "../system/color.h"
#include "../system/i2c.h"

void free_ds18(Sensor * sensor);
bool read_ds18(one_device * ds18_one);

Sensor * init_ds18(ProtoSensor * proto) {
  
  Sensor * ds18 = malloc(sizeof(*ds18));
  
  ds18 -> name = "DS18B20";
  ds18 -> free = free_ds18;
  
  ds18 -> one = create_one_device
    (ds18, proto, "/sys/devices/w1_bus_master1/****/w1_slave", "logs/ds18.log", proto -> hertz);
  
  fprintf(ds32 -> one -> log, RED "\n\nDS18B20\n Start time %s\nTemp *C\n" RESET, formatted_time);
  
  return ds18;
}

bool read_ds18(one_device * ds18_one) {
  
  if (ds18 -> should_print)
    printf("ds18: Printing not implemented\n");
  
  fprintf(ds18_one -> log, "Not implemented\n");
  return true;
}

void free_ds18(Sensor * ds18) {
  
}
