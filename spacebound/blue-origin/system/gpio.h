#pragma once

#include <stdbool.h>

typedef struct Pin {

  union {
    bool hot;
    int duty;
  };
  
  int ms_till_hot;
  int ms_till_cold;
  
} Pin;

Pin pins[40];

void init_pins();
void pin_set(char broadcom, bool hot);
void pin_set_hot (void * nil, char * vbroadcom);    // selector commands
void pin_set_cold(void * nil, char * vbroadcom);    // -----------------
