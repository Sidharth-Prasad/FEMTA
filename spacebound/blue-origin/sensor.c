

#include <stdlib.h>
#include <stdio.h>

#include "adxl.h"
#include "ds31.h"
#include "fram.h"
#include "list.h"
#include "sensor.h"


void init_sensors() {
  
  sensors = create_list(SLL, NULL);
  
  list_insert(sensors, init_adxl());
  list_insert(sensors, init_ds31());
  list_insert(sensors, init_fram());
}

void destroy_sensors() {
  
  
}

