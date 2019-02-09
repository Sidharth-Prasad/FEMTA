

#include <stdlib.h>
#include <stdio.h>

#include "sensor.h"
#include "adxl.h"
#include "list.h"


void init_sensors() {
  
  sensors = create_list(SLL, NULL);
  
  list_insert(sensors, init_adxl());
}


