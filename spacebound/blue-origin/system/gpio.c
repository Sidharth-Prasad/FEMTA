
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>

#include "gpio.h"
#include "color.h"

Pin pins[40];

void init_pins() {
  
  for (int i = 0; i < 40; i++) {
    pins[i].hot  = -1;
    pins[i].duty = -1;
  }
  
  /*gpioWrite(23, 0);
  gpioWrite(24, 0);
  gpioWrite(27, 0);*/
}

void pin_set(char broadcom, bool hot) {
  
  if (pins[broadcom].hot != hot) {
    pins[broadcom].hot = hot;
    gpioWrite(broadcom, (int) hot);
    
    printf(YELLOW "DEBUG: %d set %d\n" RESET, broadcom, hot);
  }
}
