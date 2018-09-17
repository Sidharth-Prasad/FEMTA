
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "linked-list.h"
#include "selector.h"
#include "graphics.h"


void change_selector(void * selector);   // promise

Selector * create_selector(Selector * parent) {

  Selector * selector = malloc(sizeof(Selector));
  
  selector -> parent  = parent;
  
  selector -> entries = create_list(0, false);

  if (parent) add_selector_command(selector, 'b', "back", (lambda) change_selector, (void *) parent);

  return selector;
}

void add_selector_command(Selector * selector, char key, char * text, lambda action, void * argument) {
  
  Command * command = malloc(sizeof(Command));

  command -> key  = key;
  command -> text = text;

  command -> action   = (lambda) action;
  command -> argument = argument;

  list_insert(selector -> entries, create_node((void *) command));
}

void execute_selector(Selector * selector, char key) {

  for (Node * node = selector -> entries -> head; node; node = node -> next) {

    Command * entry = (Command *) node -> value;

    if (entry -> key != key) continue;

    (entry -> action)(entry -> argument);
  }
}

void present_selector(Selector * selector) {

  erase_print_window(1);
  
  char text[64];
  
  for (Node * node = selector -> entries -> head; node; node = node -> next) {

    Command * entry = (Command *) node -> value;

    sprintf(text, "%c: %s", entry -> key, entry -> text);

    print(1, text, 0);
  }
}


// Functions we bind to selectors

void change_selector(void * selector) {
  visible_selector = (Selector *) selector;
  present_selector((Selector *) selector);
}

void flip_bool(void * pointer) {             // Flips a boolean value
  *(bool *) pointer = !(*(bool *) pointer);
}

void cycle_graph(void * list) {              // Change graph to display
  return;
}

void flip_femta(void * number) {             // Flip a FEMTA module
  return;
}

void flip_valve(void * nil) {                // Flip the valve
  return;
}
void rotate(void * nil) {                    // Rotate a number of degrees
  return;
}

void write_message(void * nil) {             // Writes a message to the log
  return;
}

