#pragma once

#include <stdbool.h>

typedef struct Pin {

  union {
    bool hot;
    int duty;
  };
  
} Pin;

void init_pins();
void pin_set(char broadcom, bool hot);
