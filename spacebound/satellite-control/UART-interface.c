#include <stdbool.h>
#include <stdlib.h>

#include "unified-controller.h"
#include "UART-interface.h"

float readSerialTempData() {
  return -0.0;
}


bool initialize_UART(module * initialent) {
  initialent -> uart = malloc(sizeof(UART));
  serial_device = initialent;
  serial_device -> uart -> temperature = &readSerialTempData;
  return true;
}
