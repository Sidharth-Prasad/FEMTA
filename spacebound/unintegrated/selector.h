#ifndef COMPILER_GAURD_SELECTOR
#define COMPILER_GAURD_SELECTOR

#include "linked-list.h"

typedef void (*lambda)(void *);

typedef struct Command {

  char   key;
  char * text;
  lambda action;
  void * argument;
  
} Command;

typedef struct Selector Selector;   // Promising definition

typedef struct Selector {

  char * title;
  
  List * entries;

  Selector * parent;

} Selector;

Selector * visible_selector;

Selector * create_selector();
void add_selector_command(Selector * selector, char key, char * text, lambda action, void * argument);
void execute_selector(Selector * selector, char key);
void present_selector(Selector * selector);

#endif
