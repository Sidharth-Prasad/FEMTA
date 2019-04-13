

#include <stdlib.h>
#include <stdio.h>

#include "ad15.h"
#include "adxl.h"
#include "ds32.h"
#include "fram.h"
#include "sensor.h"

#include "../structures/list.h"
#include "../structures/hashmap.h"

ProtoSensor * proto_sensor_create(char * code_name, int hertz) {
  
  ProtoSensor * proto = malloc(sizeof(ProtoSensor));
  
  proto -> code_name = code_name;
  proto -> hertz     = hertz;
  proto -> requested = false;
  proto -> print     = false;
  
  return proto;
}

void init_sensors() {
  
  proto_sensors = hashmap_create(hash_string, compare_strings, NULL, 8);
  
  hashmap_add(proto_sensors, "adxl", proto_sensor_create("adxl", 100));
  hashmap_add(proto_sensors, "ds32", proto_sensor_create("ds32",   1));
  hashmap_add(proto_sensors, "fram", proto_sensor_create("fram",   0));
  
  hashmap_add(proto_sensors, "ad15_gnd", proto_sensor_create("ad15_gnd", 100));
  hashmap_add(proto_sensors, "ad15_vdd", proto_sensor_create("ad15_vdd", 100));
  hashmap_add(proto_sensors, "ad15_sda", proto_sensor_create("ad15_sda", 100));
  hashmap_add(proto_sensors, "ad15_scl", proto_sensor_create("ad15_scl", 100));
}

void start_sensors() {

  sensors = list_create();
  
  ProtoSensor * proto;
  
  // ds32
  proto = hashmap_get(proto_sensors, "ds32");
  
  if (proto -> requested)
    list_insert(sensors, init_ds32(proto));    // first so we can get the time
  
  
  // adxl
  proto = hashmap_get(proto_sensors, "adxl");

  if (proto -> requested)
    list_insert(sensors, init_adxl(proto));
  
  
  // fram
  proto = hashmap_get(proto_sensors, "fram");
  
  if (proto -> requested)
    list_insert(sensors, init_fram(proto));

  
  // ad15
  
  proto = hashmap_get(proto_sensors, "ad15");
  
  if (!proto -> requested)
    return;
  
  Sensor * ad15[4] = {

    //        address              title text in log           mode
    init_ad15(AD15_GND, "Single channels", list_from(4,  A0,  A1, A2, A3), list_from(4, "A0", "A1", "A2", "A3")),
    init_ad15(AD15_VDD, "Differentials"  , list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23")),
    init_ad15(AD15_SDA, "Differentials"  , list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23")),
    init_ad15(AD15_SCL, "Differentials"  , list_from(2, A01, A23), list_from(2, "Diff 01", "Diff 23")),
  };
  
  list_insert(sensors, ad15[0]);   // single 0 -> 1
  list_insert(sensors, ad15[1]);
  list_insert(sensors, ad15[2]);
  list_insert(sensors, ad15[0]);   // single 1 -> 2
  list_insert(sensors, ad15[3]);
  list_insert(sensors, ad15[1]);
  list_insert(sensors, ad15[0]);   // single 2 -> 3
  list_insert(sensors, ad15[2]);
  list_insert(sensors, ad15[3]);
  list_insert(sensors, ad15[0]);   // single 3 -> 0
  
  /*list_insert(sensors, init_ad15(AD15_GND, "Experiment Data",
				 list_from(1, A01),
				 list_from(1, "Differential 01")));*/
  
  /*list_insert(sensors, init_ad15(AD15_GND, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));*/
  
  /*list_insert(sensors, init_ad15(AD15_VDD, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));*/

  /*list_insert(sensors, init_ad15(AD15_SDA, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));
  
  list_insert(sensors, init_ad15(AD15_SCL, "Hypothetical Sensors",
				 list_from(4, A0, A1, A2, A3),
				 list_from(4, "Analog 0", "Analog 1", "Analog 2", "Analog 3")));*/
}

void destroy_sensors() {
  
  
}

