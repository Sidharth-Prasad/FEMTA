

#include <stdlib.h>
#include <stdio.h>

#include "adxl.h"
#include "list.h"
#include "sensor.h"


void init_sensors() {
  
  sensors = create_list(SLL, NULL);
  
  list_insert(sensors, init_adxl());
}

void destroy_sensors() {
  
  
}

