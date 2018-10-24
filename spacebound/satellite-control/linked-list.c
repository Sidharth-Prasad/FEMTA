
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

  if (list -> head == NULL) {
    list -> head = node;
    list -> tail = node;
    list -> elements = 1;
    
    if (list -> doublely_linked) {
      node -> prev = node;
      node -> next = node;
    }
    return;
  }

  if (!list -> doublely_linked) {
    node -> next = list -> head;
    list -> head = node;
    list -> elements++;
    return;
  }

  // limit does not exist or has not been reached
  if (!list -> elements_limit || list -> elements < list -> elements_limit) {
    node -> next = list -> head;
    node -> prev = list -> head -> prev;
    list -> head -> prev -> next = node;
    list -> head -> prev = node;
    list -> head = node;
    list -> elements++;
    return;
  }

  // limit has been reached
  list -> head -> prev -> value = node -> value;
  list -> head = list -> head -> prev;

  // Delete the new node
  free(node);
}

void list_insert_last(List * list, void * datum) {
  // Inserts new node after the tail
  // Currently requires the list be an SLL!
  
  Node * node = create_node(datum);

  if (!list -> head) {
    list -> head = node;          // First element, so set 
    list -> tail = node;          // -- head and tail here
    return;
  }

  list -> tail -> next = node;
  list -> tail = node;
}

void list_remove(List * list, Node * node) {
  // Removes node from list
  // Complexity: O(n) for SLLs
  // Complexity: O(1) for DLLs
  // Note: I have not considered limited DLLs

  if (!list -> doublely_linked) {
    // Seek for previous node
    
    Node * previous;
    
    for (previous = list -> head; previous != node; previous = previous -> next) {
      if (previous -> next == node) break;
    }
    
    previous -> next = node -> next;   // Drop out of SLL

    if (node == list -> tail) {
      list -> tail = previous;         // Retract tail
    }
  }

  else {
    node -> next -> prev = node -> prev;   // Drop out of DLL
    node -> prev -> next = node -> next;   // ---------------
  }
  
  if (node == list -> head) {
    list -> head = node -> next;       // Advance head
  }
  
  if (--list -> elements == 0) list -> head = NULL;
  if (list -> auto_free) free(node -> value);
  free(node);
}


void list_destroy(List * list) {

  while (list -> elements) {
    list_remove(list, list -> head);
  }

  free(list);
}
