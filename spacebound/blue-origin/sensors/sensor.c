

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ad15.h"
#include "adxl.h"
#include "ds18.h"
#include "ds32.h"
#include "fram.h"
#include "sensor.h"

#include "../math/mathematics.h"
#include "../math/units.h"
#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/class.h"
#include "../system/color.h"

void sensor_call_free(void *);

ProtoSensor * proto_sensor_create(char * code_name, int address, Hashmap * targets, int bus) {
  
  ProtoSensor * proto = calloc(1, sizeof(*proto));
  
  proto -> code_name = code_name;
  proto -> address   = address;
  proto -> targets   = targets;
  proto -> bus       = bus;
  
  proto -> print_code  = RESET;
  proto -> print_hertz = 5;
  
  if (targets && targets -> size)
    proto -> output_paths = calloc(targets -> elements, sizeof(*proto -> output_paths));
  else
    proto -> output_paths = NULL;
  
  return proto;
}

Sensor * sensor_from_proto(ProtoSensor * proto) {
  
  Sensor * sensor = calloc(1, sizeof(*sensor));
  
  sensor -> bus             = proto -> bus;
  sensor -> print           = proto -> print;
  sensor -> targets         = proto -> targets;
  sensor -> triggers        = proto -> triggers;
  sensor -> code_name       = proto -> code_name;
  sensor -> print_code      = proto -> print_code;
  sensor -> print_hertz     = proto -> print_hertz;
  sensor -> output_paths    = proto -> output_paths;
  sensor -> auto_regressive = proto -> auto_regressive;
  
  return sensor;
}

void init_sensors() {
  
  n_triggers = 0;
  
  sprintf(formatted_time, "[Clock not present!]");    // overwritten by clock
  
  proto_sensors = hashmap_create(hash_string, compare_strings, NULL, 8);
  
  Hashmap * adxl_tar = hashmap_create(hash_string, compare_strings, NULL, 3);
  Hashmap * ad15_tar = hashmap_create(hash_string, compare_strings, NULL, 6);
  Hashmap * ds32_tar = hashmap_create(hash_string, compare_strings, NULL, 1);
  Hashmap * ds18_tar = hashmap_create(hash_string, compare_strings, NULL, 1);
  Hashmap * fram_tar = NULL;
  
  hashmap_add(ds32_tar, "Time"       , (void *) (int) 0);
  hashmap_add(ds32_tar, "Temperature", (void *) (int) 1);
  
  hashmap_add(ds18_tar, "Temperature", (void *) (int) 0);
  
  hashmap_add(adxl_tar, "X", (void *) (int) 0);
  hashmap_add(adxl_tar, "Y", (void *) (int) 1);
  hashmap_add(adxl_tar, "Z", (void *) (int) 2);
  
  hashmap_add(ad15_tar, "A01", (void *) (int) 0);    // if this doesn't make sense to you,
  hashmap_add(ad15_tar, "A23", (void *) (int) 1);    // really think it through cause it's 
  hashmap_add(ad15_tar, "A0" , (void *) (int) 0);    // critical to the ad15's interface
  hashmap_add(ad15_tar, "A1" , (void *) (int) 1);    // 
  hashmap_add(ad15_tar, "A2" , (void *) (int) 2);    // 
  hashmap_add(ad15_tar, "A3" , (void *) (int) 3);    // 
  
  
  hashmap_add(proto_sensors, "adxl"    , proto_sensor_create("adxl", ADXL_ADDRESS, adxl_tar, I2C_BUS));
  hashmap_add(proto_sensors, "ds32"    , proto_sensor_create("ds32", DS32_ADDRESS, ds32_tar, I2C_BUS));
  hashmap_add(proto_sensors, "fram"    , proto_sensor_create("fram", FRAM_ADDRESS, fram_tar, I2C_BUS));
  hashmap_add(proto_sensors, "ds18"    , proto_sensor_create("ds18",            0, ds18_tar, ONE_BUS));
  hashmap_add(proto_sensors, "ad15_gnd", proto_sensor_create("ad15_gnd", AD15_GND, ad15_tar, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_vdd", proto_sensor_create("ad15_vdd", AD15_VDD, ad15_tar, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_sda", proto_sensor_create("ad15_sda", AD15_SDA, ad15_tar, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_scl", proto_sensor_create("ad15_scl", AD15_SCL, ad15_tar, I2C_BUS));
}

void start_sensors() {
  
  int i2c_divisor = -1;
  int one_divisor = -1;
  
  for (iterate(proto_sensors -> all, ProtoSensor *, proto)) {
    
    if (!proto -> requested || !proto -> hertz) continue;
    
    int interval = 1000 / proto -> hertz;
    
    if (interval * proto -> hertz != 1000) {
      printf(RED "\n1000ms is not cleanly divided by %s's %dHz\n\n" RESET, proto -> code_name, proto -> hertz);
      exit(1);
    }

    switch (proto -> bus) {
      
    case I2C_BUS:
      if (i2c_divisor == -1) i2c_divisor = interval;
      i2c_divisor = gcd(interval, i2c_divisor);
      break;
      
    case ONE_BUS:
      if (one_divisor == -1) one_divisor = interval;
      one_divisor = gcd(interval, one_divisor);
      break;
      
    default:
      printf(RED "Bus type %x not supported\n" RESET, proto -> bus);
      break;
    }
  }
  
  
  
  if (i2c_divisor != -1) {
    
    printf("\nBuilding I2C sensor schedule using " GREEN "%dms" RESET " intervals\n\n", i2c_divisor);
    
    if (i2c_divisor < 10)
      printf(RED "I2C schedule with divisor %dms is likely unserviceable.\n"
             "Please consider more harmonious frequencies from below\n" RESET
             "\t 1, 2, 4, 5, 10, 20, 25, 40, 50, 100, 125, 200, 250\n\n", i2c_divisor);
    
    schedule -> i2c_active = true;
  }
  
  if (one_divisor != -1) {
    
    printf("\nBuilding 1-wire sensor schedule using " GREEN "%dms" RESET " intervals\n\n", one_divisor);
    
    if (one_divisor < 10)
      printf(RED "1-Wire schedule with divisor %dms is likely unserviceable.\n"
             "Please consider more harmonious frequencies from below\n" RESET
             "\t 1, 2, 4, 5, 10, 20, 25, 40, 50, 100, 125, 200, 250\n\n", one_divisor);

    schedule -> one_active = true;
  }
  
  schedule -> i2c_interval = i2c_divisor * 1E6;
  schedule -> one_interval = one_divisor * 1E6;
  
  sensors = list_that_frees(sensor_call_free);
  
  
  
  ProtoSensor * proto;

  /* i2c sensors */
  
  // ds32
  proto = hashmap_get(proto_sensors, "ds32");
  
  if (proto -> requested) {
    Sensor * ds32 = init_ds32(proto);
    list_insert(sensors,                 ds32       );
    list_insert(schedule -> i2c_devices, ds32 -> i2c);    // first so we can get the time
  }
  
  
  // adxl
  proto = hashmap_get(proto_sensors, "adxl");

  if (proto -> requested) {
    Sensor * adxl = init_adxl(proto);
    list_insert(sensors,                 adxl       );
    list_insert(schedule -> i2c_devices, adxl -> i2c);
  }
  
  
  // fram
  proto = hashmap_get(proto_sensors, "fram");
  
  if (proto -> requested) {
    Sensor * fram = init_fram(proto);
    list_insert(sensors,                 fram       );
    list_insert(schedule -> i2c_devices, fram -> i2c);
  }

  
  // ad15
  Sensor * ad15[4] = {
    NULL, NULL, NULL, NULL,
  };

  proto = hashmap_get(proto_sensors, "ad15_gnd");
  
  if (proto -> requested) {
    ad15[0] = init_ad15(proto, "Single channels",
			list_from(4, A0, A1, A2, A3),
			list_from(4, "10kOhm", "+3.3V", "Thermister 2", "Thermister 1"));

    list_insert(sensors, ad15[0]);
  }

  proto = hashmap_get(proto_sensors, "ad15_vdd");

  if (proto -> requested) {
    ad15[1] = init_ad15(proto, "Alcohol Pressure",
			list_from(4, A0, A1, A2, A3),
			list_from(4, "ground", "+5V", "Thermister 3", "Thermister 6"));
    
    list_insert(sensors, ad15[1]);
  }

  proto = hashmap_get(proto_sensors, "ad15_sda");

  if (proto -> requested) {
    //ad15[2] = init_ad15(proto, "Alcohol Pressure", list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23"));
    ad15[2] = init_ad15(proto, "Ambient Air",
			list_from(4, A0, A1, A2, A3),
			list_from(4, "Thermister 5", "Thermister 4", "Thermister 7", "Thermister 8"));
    
    list_insert(sensors, ad15[2]);
  }

  proto = hashmap_get(proto_sensors, "ad15_scl");

  if (proto -> requested) {
    ad15[3] = init_ad15(proto, "Differentials",
			list_from(4, A0, A1, A2, A3),
			list_from(4, "Thermister 9", "Thermister 10", "Thermister 11", "Thermister 12"));
    list_insert(sensors, ad15[3]);
  }
  
  /*
  if (ad15[0]) list_insert(schedule -> i2c_devices, ad15[0] -> i2c);   // single 0 -> 1
  if (ad15[1]) list_insert(schedule -> i2c_devices, ad15[1] -> i2c);
  if (ad15[2]) list_insert(schedule -> i2c_devices, ad15[2] -> i2c);
  if (ad15[0]) list_insert(schedule -> i2c_devices, ad15[0] -> i2c);   // single 1 -> 2
  if (ad15[3]) list_insert(schedule -> i2c_devices, ad15[3] -> i2c);
  if (ad15[1]) list_insert(schedule -> i2c_devices, ad15[1] -> i2c);
  if (ad15[0]) list_insert(schedule -> i2c_devices, ad15[0] -> i2c);   // single 2 -> 3
  if (ad15[2]) list_insert(schedule -> i2c_devices, ad15[2] -> i2c);
  if (ad15[3]) list_insert(schedule -> i2c_devices, ad15[3] -> i2c);
  if (ad15[0]) list_insert(schedule -> i2c_devices, ad15[0] -> i2c);   // single 3 -> 0
  */
  
  for (int channel = 0; channel < 4; channel++)
    for (int sensor_index = 0; sensor_index < 4; sensor_index++)
      if (ad15[sensor_index])
	list_insert(schedule -> i2c_devices, ad15[sensor_index] -> i2c);
  
  
  /* 1-wire sensors */
  
  // ds18
  proto = hashmap_get(proto_sensors, "ds18");
  
  if (proto -> requested) {
    Sensor * ds18 = init_ds18(proto);
    list_insert(sensors,                 ds18       );
    list_insert(schedule -> one_devices, ds18 -> one);
  }
}

float time_passed() {
  return (schedule -> interrupts) * (schedule -> interrupt_interval);
}

void sensor_call_free(void * vsensor) {
  
  Sensor * sensor = vsensor;
  
  printf(YELLOW "Removing sensor %s\n" RESET, sensor -> name);
  
  if      (sensor -> bus == I2C_BUS) i2c_close(sensor -> i2c);
  else if (sensor -> bus == ONE_BUS) one_close(sensor -> one);
  $(sensor, free);
}

void terminate_sensors() {  
  list_destroy(sensors);
  sensors = NULL;
}

void flip_print(void * nil, char * raw_text) {
  // toggle sensor printing for the sensor specified
  
  if (strlen(raw_text) < 3) return;  // no such sensor
  
  raw_text[strlen(raw_text) - 1] = '\0';
  
  for (iterate(sensors, Sensor *, sensor))
    if (!strcmp(sensor -> code_name, raw_text + 2))
      sensor -> print = !sensor -> print;
}
