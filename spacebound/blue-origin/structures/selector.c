
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "list.h"
#include "selector.h"


void change_selector(void * selector);

Selector * create_selector(Selector * parent) {

  Selector * selector = malloc(sizeof(Selector));
  
  selector -> parent  = parent;
  
  selector -> entries = list_create();
  
  return selector;
}

void add_selector_command(Selector * selector, char key, char * text, slambda action, void * argument) {
  
  Command * command = malloc(sizeof(Command));

  command -> key  = key;
  command -> text = text;

  command -> action   = action;
  command -> argument = argument;

  list_insert(selector -> entries, command);
}

void execute_selector(Selector * selector,     // the selector structure
		      char key,                // the command identifier
		      char * raw_text          // the raw text entered
		      ) {
  
  for (iterate(selector -> entries, Command *, entry)) {
    
    if (entry -> key != key) continue;
    
    (entry -> action)(entry -> argument, raw_text);
  }
}

void output_str(void * nil, char * raw_text) {
  /* print a message to the console */
  
  if (strlen(raw_text) < 2) printf("user message: \n");
  else                      printf("user message: %s\n", raw_text + 1);
}

void flip_bool(void * pointer, char * raw_text) {    /* Flips a boolean value */
  *(bool *) pointer = !(*(bool *) pointer);
}
