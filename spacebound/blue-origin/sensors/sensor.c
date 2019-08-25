

#include <stdlib.h>
#include <stdio.h>

#include "ad15.h"
#include "adxl.h"
#include "ds32.h"
#include "fram.h"
#include "sensor.h"

#include "../structures/list.h"
#include "../structures/hashmap.h"
#include "../system/color.h"
#include "../math/mathematics.h"


ProtoSensor * proto_sensor_create(char * code_name, int hertz, int address, Hashmap * targets, List * betas, int bus) {
  
  ProtoSensor * proto = malloc(sizeof(ProtoSensor));
  
  proto -> code_name = code_name;
  proto -> hertz     = hertz;
  proto -> address   = address;
  proto -> targets   = targets;
  proto -> betas     = betas;
  proto -> bus       = bus;
  proto -> triggers  = NULL;
  proto -> requested = false;
  proto -> print     = false;
  
  return proto;
}

void init_sensors() {
  
  sprintf(formatted_time, "[Clock not present!]");    // overwritten by clock
  
  proto_sensors = hashmap_create(hash_string, compare_strings, NULL, 8);
  
  Hashmap * adxl_tar = hashmap_create(hash_string, compare_strings, NULL, 3);
  Hashmap * ad15_tar = hashmap_create(hash_string, compare_strings, NULL, 6);
  Hashmap * ds32_tar = hashmap_create(hash_string, compare_strings, NULL, 1);
  Hashmap * ds18_tar = hashmap_create(hash_string, compare_strings, NULL, 1);
  Hashmap * fram_tar = NULL;
  
  hashmap_add(ds32_tar, "Time", (void *) (int) 0);
  hashmap_add(ds18_tar, "Temperature", (void *) (int) 0);
  
  hashmap_add(adxl_tar, "X", (void *) (int) 0);
  hashmap_add(adxl_tar, "Y", (void *) (int) 1);
  hashmap_add(adxl_tar, "Z", (void *) (int) 2);
  
  
  
  List * adxl_betas = list_from(6, 0.004F, 0.004F, 0.004F, -0.0371F, 0.0010F, 0.0861F); 
  
  hashmap_add(proto_sensors, "adxl", proto_sensor_create("adxl", 200, ADXL_ADDRESS, adxl_tar, adxl_betas, I2C_BUS));
  hashmap_add(proto_sensors, "ds32", proto_sensor_create("ds32",   1, DS32_ADDRESS, ds32_tar, NULL      , I2C_BUS));
  hashmap_add(proto_sensors, "fram", proto_sensor_create("fram",   0, FRAM_ADDRESS, fram_tar, NULL      , I2C_BUS));
  hashmap_add(proto_sensors, "ds18", proto_sensor_create("ds18",   0,            0, ds18_tar, NULL      , ONE_BUS));
  
  hashmap_add(ad15_tar, "A01", (void *) (int) 0);
  hashmap_add(ad15_tar, "A23", (void *) (int) 1);
  hashmap_add(ad15_tar, "A0" , (void *) (int) 0);
  hashmap_add(ad15_tar, "A1" , (void *) (int) 1);
  hashmap_add(ad15_tar, "A2" , (void *) (int) 2);
  hashmap_add(ad15_tar, "A3" , (void *) (int) 3); 
  
  List * gnd_betas = list_from(2, 0.0001874118F, -0.0009114640F);
  List * vdd_betas = list_from(2, 0.0001874287F, -0.0009012627F);
  List * sda_betas = list_from(2, 0.0001873591F, -0.0005211696F);
  List * scl_betas = list_from(2, 0.0001873755F, -0.0009628627F);
  
  hashmap_add(proto_sensors, "ad15_gnd", proto_sensor_create("ad15_gnd", 0, AD15_GND, ad15_tar, gnd_betas, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_vdd", proto_sensor_create("ad15_vdd", 0, AD15_VDD, ad15_tar, vdd_betas, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_sda", proto_sensor_create("ad15_sda", 0, AD15_SDA, ad15_tar, sda_betas, I2C_BUS));
  hashmap_add(proto_sensors, "ad15_scl", proto_sensor_create("ad15_scl", 0, AD15_SCL, ad15_tar, scl_betas, I2C_BUS));
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
  
  sensors = list_create();
  
  
  
  ProtoSensor * proto;
  
  // ds32
  proto = hashmap_get(proto_sensors, "ds32");
  
  if (proto -> requested) {
    Sensor * ds32 = init_ds32(proto);
    list_insert(sensors,                 ds32       );    // first so we can get the time
    list_insert(schedule -> i2c_devices, ds32 -> i2c);
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
    ad15[0] = init_ad15(proto, "Single channels", list_from(4, A0, A1, A2, A3), list_from(4, "A0", "A1", "A2", "A3"));
    list_insert(sensors, ad15[0]);
  }

  proto = hashmap_get(proto_sensors, "ad15_vdd");

  if (proto -> requested) {
    ad15[1] = init_ad15(proto, "Alcohol Pressure", list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23"));
    list_insert(sensors, ad15[1]);
  }

  proto = hashmap_get(proto_sensors, "ad15_sda");

  if (proto -> requested) {
    ad15[2] = init_ad15(proto, "Ambient Air", list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23"));
    list_insert(sensors, ad15[2]);
  }

  proto = hashmap_get(proto_sensors, "ad15_scl");

  if (proto -> requested) {
    ad15[3] = init_ad15(proto, "Differentials", list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23"));    
    list_insert(sensors, ad15[3]);
  }
  
  
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
  
  
  
  // ds18
  proto = hashmap_get(proto_sensors, "ds18");
  
  if (proto -> requested) {
    Sensor * ds18 = init_ds18(proto);
    list_insert(sensors,                 ds18       );    // first so we can get the time
    list_insert(schedule -> one_devices, ds18 -> one);
  }
}

void destroy_sensors() {
  // TODO
}
