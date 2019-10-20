#ifndef HEADER_GAURD_STATE
#define HEADER_GAURD_STATE

#include <stdbool.h>

void init_states();
void drop_states();

void add_state(char * state_name, bool entered);
bool state_exists(char * state_name);

void enter(char * state_name);
void leave(char * state_name);

bool state_get(char * state_name);

#endif


