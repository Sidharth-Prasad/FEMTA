
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "linked-list.h"
#include "selector.h"
#include "graphics.h"
#include "femta.h"

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

void cycle_graph(void * nil) {              // Change graph to display

  // Add MPU plots
  if (MPU -> initialized && !MPU -> loaded) {
    for (int p = 1; p <= 3; p++) list_insert(owner_index_list, create_node((void *) p));
    MPU -> loaded = true;
    print(GENERAL_WINDOW, "Loaded MPU plots", 1);
  }

  graph_owner_index_node = graph_owner_index_node -> next;
  graph_owner = all_possible_owners[(int) (graph_owner_index_node -> value)];
  
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

void write_message(void * logger) {
  // Writes a message to the log
  // Gets input from the user in the control window

  erase_print_window(1);
  print(CONTROL_WINDOW, "Please enter a message:", 5);
  print(CONTROL_WINDOW, "", 1);                          // Print an empty line
  
  char message[32];
  
  echo();
  //mvwgetstr(print_views[CONTROL_WINDOW] -> view -> window, 0, 0, message);
  wgetstr(print_views[CONTROL_WINDOW] -> view -> window, message);
  noecho();

  print(GENERAL_WINDOW, "Logged message:", 5);
  print(GENERAL_WINDOW, message, 1);

  // Get timestamp
  int mpu_reads = 0;
  if (mpu_logger) mpu_reads = mpu_logger -> values_read;
  int relativeTime = time(NULL) - start_time;

  // Log the message
  Logger * log = (Logger *) logger;
  
  log -> open(log);
  fprintf(log -> file, "%d\t%d\t%s\n", mpu_reads, relativeTime, message);
  log -> close(log);

  present_selector((void *) visible_selector);
}

