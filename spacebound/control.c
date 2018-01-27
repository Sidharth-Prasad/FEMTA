
#include <stdlib.h>
#include <stdio.h>

typedef struct module {

  int logical;
  int physical;
  int state;
  int pwm;
  
} module;



int main() {

  module * BNO   = malloc(    sizeof(module));
  module * MPU   = malloc(    sizeof(module));
  module * Valve = malloc(2 * sizeof(module));
  module * FEMTA = malloc(4 * sizeof(module));

  

  return 0;
}
