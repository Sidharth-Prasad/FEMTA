#include <stdlib.h>
#include <pigpio.h>
#include <stdio.h>

int main(void) {
  
  typedef int *Acc;      // Defino un handle (Apuntador) // not sure about this one
  gpioInitialise();      // inicializa la libreria
  if (gpioInitialise() <=0) {
    printf("gpio not initialise\n");
    return 0;
  }
  
   

  int sensor = i2cOpen(1, 0x68, 0);

  printf("Sensor:\t%d\n", sensor);

  for (int i = 0; i < 256; i++) {
    int data = i2cReadByteData(sensor, i);
    if (data != PI_I2C_READ_FAILED) printf("Block %d:\t%d\n", i, data);
    
  }
  
  /*printf("%d, %d, %d\n", PI_BAD_HANDLE, PI_BAD_PARAM, PI_I2C_READ_FAILED);

  for (int s = 0; s < 64; s++) {
    int sensor = i2cOpen(1, s, 0);
    printf("Sensor:\t%d\n", sensor);
    for (int i = 0; i < 256; i++)  {
      int data = i2cReadByteData(sensor, i);
      if (data != PI_I2C_READ_FAILED) printf("Block %d:\t%d\n", i, data);
    }
    }*/

  gpioTerminate();
  return 0;
}
