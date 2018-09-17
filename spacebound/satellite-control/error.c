
#include <stdlib.h>
#include <stdio.h>

#include "colors.h"
#include "error.h"

void exit_printing(char * message, char code) {

  printf(CONSOLE_RED "%s\n" CONSOLE_RESET, message);
  exit(code);
  
}
