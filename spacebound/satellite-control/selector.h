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
void     execute_selector(Selector * selector, char key);
void     present_selector(Selector * selector);

// All functions we bind to selectors in femta.c
void change_selector(void * selector);      // Switches visible selector
void flip_bool(void * pointer);             // Flips a boolean value
void cycle_graph(void * nil);               // Change graph to display
void flip_femta(void * number);             // Flip a FEMTA module
void flip_valve(void *    nil);             // Flip the valve
void rotate(void * nil);                    // Rotate a number of degrees
void write_message(void * nil);             // Writes a message to the log



#endif
