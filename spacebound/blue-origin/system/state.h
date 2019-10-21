#ifndef HEADER_GAURD_STATE
#define HEADER_GAURD_STATE

#include <stdbool.h>

typedef struct Transition {
  
  char * state;    // the broadcom number of the pin
  int    delay;    // used for delayed state changes
  
} Transition;

typedef struct StateDelay {

  char * state;           // the state whose transition is under this delay
  int    ms_remaining;    // remaining ms before state change
  bool   entering;        // whether state should be entered
  
} StateDelay;

Hashmap * state_delays;

void init_states();
void drop_states();
void print_all_states();

void add_state(char * state, bool entered);
bool state_exists(char * state);

void enter_state(char * state);
void leave_state(char * state);

void enter(Transition * trans);
void leave(Transition * trans);

bool state_get(char * state);

Transition * transition_create(char * state, int delay);
void state_inform_delays(char * state);

#endif
