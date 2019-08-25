#ifndef COMPILER_GAURD_SELECTOR
#define COMPILER_GAURD_SELECTOR

#include <stdbool.h>

#include "list.h"

#include "../types/types.h"

typedef struct Command {
  
  char   key;
  char * text;
  lambda action;
  void * argument;
  
} Command;

typedef struct Selector {

  char * title;
  
  List * entries;
  
  struct Selector * parent;

} Selector;

bool reading_user_input;

Selector * create_selector();

void add_selector_command(Selector * selector, char key, char * text, lambda action, void * argument);
void     execute_selector(Selector * selector, char key);

void output_str(void * str_ptr);
void flip_bool(void * pointer);

#endif
