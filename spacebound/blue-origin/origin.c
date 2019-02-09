

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "list.h"
#include "types.h"
#include "clock.h"
#include "i2c.h"




int main() {
  
  init_sensors();
  
  init_i2c();

  sleep(1);
  schedule -> term_signal = true;
  pthread_join(*schedule -> thread, NULL);
  
  return EXIT_SUCCESS;
}
