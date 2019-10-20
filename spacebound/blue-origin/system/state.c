
#include <stdbool.h>

#include "state.h"

#include "../structures/hashmap.h"

static Hashmap * states;    // maps string names to bools

void init_states() {
  states = hashmap_create(hash_string, compare_strings, key_free, 16);
}

void drop_states() {
  hashmap_destroy(states);
}

void add_state(char * state_name, bool entered) {
  hashmap_add(states, state_name, (void *) entered);
}

bool state_exists(char * state_name) {
  return hashmap_exists(states, state_name);
}

void enter(char * state_name) {
  hashmap_update(states, state_name, (void *) true);
}

void leave(char * state_name) {
  hashmap_update(states, state_name, (void *) false);
}

bool state_get(char * state_name) {
  return (bool) hashmap_get(states, state_name);
}
