
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "list.h"
#include "selector.h"


void change_selector(void * selector);   // promise

Selector * create_selector(Selector * parent) {

  Selector * selector = malloc(sizeof(Selector));
  
  selector -> parent  = parent;
  
  selector -> entries = list_create();
  
  return selector;
}

void add_selector_command(Selector * selector, char key, char * text, lambda action, void * argument) {
  
  Command * command = malloc(sizeof(Command));

  command -> key  = key;
  command -> text = text;

  command -> action   = (lambda) action;
  command -> argument = argument;

  list_insert(selector -> entries, command);
}

void execute_selector(Selector * selector, char key) {

  for (iterate(selector -> entries, Command *, entry)) {

    if (entry -> key != key) continue;

    (entry -> action)(entry -> argument);
  }
}

void output_str(void * str_ptr) {
  
  char * output = (char *) str_ptr;
  
  printf("%s\n", output);
}

void flip_bool(void * pointer) {             // Flips a boolean value
  *(bool *) pointer = !(*(bool *) pointer);
}
