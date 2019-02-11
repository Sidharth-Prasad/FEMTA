
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "list.h"
#include "selector.h"


void change_selector(void * selector);   // promise

Selector * create_selector(Selector * parent) {

  Selector * selector = malloc(sizeof(Selector));
  
  selector -> parent  = parent;
  
  selector -> entries = create_list(0, false, false);

  // Adds a back button if this isn't the main menu
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

void output_str(void * str_ptr) {
  
  char output = (char *) str_ptr;
  
  printf("%s\n", output);
}

void flip_bool(void * pointer) {             // Flips a boolean value
  *(bool *) pointer = !(*(bool *) pointer);
}
