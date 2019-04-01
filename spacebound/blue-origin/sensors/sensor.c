

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
  list_insert(sensors, init_adxl());
  list_insert(sensors, init_fram());
    
  list_insert(sensors, init_ad15(AD15_GND, "Hypothetical Sensors",
				 list_from(2, A01, A23),
				 list_from(2, "Differential 0", "Differential 1")));
  
  list_insert(sensors, init_ad15(AD15_VDD, "Hypothetical Sensors",
				 list_from(2, A0, A2),
				 list_from(2, "Analog 0", "Analog 2")));
  
  list_insert(sensors, init_ad15(AD15_SDA, "Hypothetical Sensors",
				 list_from(4, A0, A1, A2, A3),
				 list_from(4, "Analog 0", "Analog 1", "Analog 2", "Analog 3")));
  
  list_insert(sensors, init_ad15(AD15_SCL, "Hypothetical Sensors",
				 list_from(3, A01, A2, A3),
				 list_from(3, "Differential 0", "Analog 2", "Analog 3")));
}

void destroy_sensors() {
  
  
}

