

#include <stdio.h>
#include <pigpio.h>





int main() {

  printf("Pi no handle:           %d\n", PI_NO_HANDLE);
  printf("Pi bad handle:          %d\n", PI_BAD_HANDLE);
  printf("Pi bad param:           %d\n", PI_BAD_PARAM);
  printf("Pi serial open failed:  %d\n", PI_SER_OPEN_FAILED);
  printf("Pi serial write failed: %d\n", PI_SER_WRITE_FAILED);
  printf("Pi serial no read data: %d\n", PI_SER_READ_NO_DATA);
  printf("Pi serial read failed:  %d\n", PI_SER_READ_FAILED);
}
