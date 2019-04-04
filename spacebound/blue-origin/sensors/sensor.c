

#include <stdlib.h>
#include <stdio.h>

#include "ad15.h"
#include "adxl.h"
#include "ds32.h"
#include "fram.h"
#include "sensor.h"

#include "../structures/list.h"

void init_sensors() {
  
  sensors = list_create();
  
  list_insert(sensors, init_ds32());    // first so we can get the time
  //list_insert(sensors, init_adxl());
  //list_insert(sensors, init_fram());
  
  /*Sensor * ad15[4] = {
    ad15_create(AD15_GND, "Hypothetical Sensors on GND", A01);
    ad15_create(AD15_VDD, "Hypothetical Sensors on VDD", A01);
    ad15_create(AD15_SDA, "Hypothetical Sensors on SDA", A01);
    ad15_create(AD15_SCL, "Hypothetical Sensors on SCL", A0 );
  };
  
  list_insert(sensors, ad15_next(ad15[0], A23));    // second is what's switched to
  list_insert(sensors, ad15_next(ad15[1], A23));
  list_insert(sensors, ad15_next(ad15[2], A23));
  list_insert(sensors, ad15_next(ad15[3], A23));
  list_insert(sensors, ad15_next(ad15[0], A01));
  list_insert(sensors, ad15_next(ad15[1], A01));
  list_insert(sensors, ad15_next(ad15[2], A01));
  list_insert(sensors, ad15_next(ad15[3], A01));*/
  
  /*list_insert(sensors, init_ad15(AD15_GND, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));*/
  
  list_insert(sensors, init_ad15(AD15_VDD, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));

  /*list_insert(sensors, init_ad15(AD15_SDA, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 01", "Differential 23")));
  
  list_insert(sensors, init_ad15(AD15_SCL, "Hypothetical Sensors",
				 list_from(4, A0, A1, A2, A3),
				 list_from(4, "Analog 0", "Analog 1", "Analog 2", "Analog 3")));*/
}

void destroy_sensors() {
  
  
}

