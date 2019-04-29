
#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>

int main() {
  
  gpioInitialise();
  
  gpioWrite(23, 0);
  gpioWrite(24, 0);
}
