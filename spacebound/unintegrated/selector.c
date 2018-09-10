
#include <stdlib.h>
#include <stdio.h>

#include "linked-list.h"
#include "selector.h"


void switch_to_selector(void * selector);   // promise

Selector * create_selector(Selector * parent) {

  Selector * selector = malloc(sizeof(Selector));
  
  selector -> parent  = parent;
  
  selector -> entries = create_list(0, false);

  if (parent) add_selector_command(selector, 'b', "back", (lambda) switch_to_selector, (void *) parent);

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

void message(void * message) {
  printf("%s\n", (char *) message);
}

void static_message(void * nothing) {
  printf("Humanity\n");
}

void switch_to_selector(void * selector) {
  printf("\nSwitching to selector %p\n", (Selector *) selector);
  visible_selector = (Selector *) selector;
}

int main() {

  Selector * main_menu = create_selector(NULL);

  visible_selector = main_menu;
  
  add_selector_command(visible_selector, 'm', "Call  message", (lambda) message      , "Main menu");
  add_selector_command(visible_selector, 'h', "Call humanity", (lambda) static_message, NULL);

  execute_selector(visible_selector, 'a');   // should do nothing
  execute_selector(visible_selector, 'm');
  execute_selector(visible_selector, 'q');   // should do nothing
  execute_selector(visible_selector, 'h');
  execute_selector(visible_selector, 'm');
  execute_selector(visible_selector, 'h');
  
  add_selector_command(visible_selector, 'a', "Again", (lambda) message, "Again main menu");
  
  execute_selector(visible_selector, 'a');   // now should print
  execute_selector(visible_selector, 'a');   // ----------------

  Selector * sub_menu = create_selector(main_menu);
  
  add_selector_command(visible_selector, 's', "Switch menus", (lambda) switch_to_selector, (void *) sub_menu);

  execute_selector(visible_selector, 's');   // switch menus

  execute_selector(visible_selector, 'm');   // should do nothing
  execute_selector(visible_selector, 'h');   // -----------------
  execute_selector(visible_selector, 'a');   // -----------------

  add_selector_command(sub_menu, 'c', "Call message", (lambda) message, "Sub menu");

  execute_selector(visible_selector, 'm');   
  execute_selector(visible_selector, 'c');   // only this should print
  execute_selector(visible_selector, 'a');
  
  execute_selector(visible_selector, 'b');   // back to main menu

  execute_selector(visible_selector, 'm');   // should print again
  execute_selector(visible_selector, 'c');   // ------------------
  execute_selector(visible_selector, 'a');   // ------------------

  execute_selector(visible_selector, 'b');   // should do nothing
  
}
