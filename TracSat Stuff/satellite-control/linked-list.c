
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "linked-list.h"

Node * create_node(void * value) {
  Node * node = malloc(sizeof(Node));
  node -> value = value;
  node -> next = NULL;
  node -> prev = NULL;
  return node;
}

List * create_list(uint limit, bool doublely_linked, bool auto_free) {
  // Creates a list with potentially limited length
  
  List * list = malloc(sizeof(List));
  list -> head = NULL;
  list -> elements = 0;
  list -> elements_limit = limit;
  list -> doublely_linked = doublely_linked;
  list -> auto_free = auto_free;
  return list;
}

List * create_list_from(uint vargs, ...) {
  // Creates a list from arguments
  // The list returned is an SLL with no auto freeing

  List * list = create_list(0, false, false);
  
  va_list argPointer;
  
  va_start(argPointer, vargs);

  for (int i = 0; i < vargs; i++) {
    list_insert_last(list, va_arg(argPointer, void *));
  }
  
  va_end(argPointer);
  
  return list;
}

void list_insert(List * list, Node * node) {
  // Inserts node into the linked list
  // Complexity: O(1)
  
  list -> elements++;
  
  if (list -> head == NULL) {
    list -> head = node;
    return;
  }
  
  node -> siblings
}
