#ifndef COMPILER_GAURD_SELECTOR
#define COMPILER_GAURD_SELECTOR

#include <stdbool.h>

#include "list.h"

#include "../types/types.h"

typedef void (* slambda)(void *, char * raw_text);

typedef struct Command {
  
  char   key;
  char * text;
  slambda action;
  void * argument;
  
} Command;

typedef struct Selector {

  char * title;
  
  List * entries;
  
  struct Selector * parent;

} Selector;

bool reading_user_input;

Selector * create_selector();

void add_selector_command(Selector * selector, char key, char * text, slambda action, void * argument);
void     execute_selector(Selector * selector, char key, char * raw_text);

void output_str(void * nil, char * raw_text);
void flip_bool(void * pointer, char * raw_text);

#endif
