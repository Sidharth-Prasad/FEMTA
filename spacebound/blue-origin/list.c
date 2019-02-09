
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "list.h"

Node * create_node(void * value) {
  Node * node = malloc(sizeof(Node));
  node -> value = value;
  node -> next = NULL;
  node -> prev = NULL;
  return node;
}

List * create_list(bool doubly_linked, lambda freer) {
  // creates an empty list
  
  List * list = malloc(sizeof(List));
  list -> head = NULL;
  list -> tail = NULL;
  list -> elements = 0;
  list -> doubly_linked = doubly_linked;
  list -> free = freer;
  return list;
}

List * create_list_from(bool doubly_linked, lambda freer, uint vargs, ...) {
  // Creates a list from arguments
  
  List * list = create_list(doubly_linked, free);
  
  va_list argPointer;
  
  va_start(argPointer, vargs);
  
  for (int i = 0; i < vargs; i++) {
    list_insert_last(list, va_arg(argPointer, void *));
  }
  
  va_end(argPointer);
  
  return list;
}

void list_insert(List * list, void * datum) {
  // Inserts node into the linked list
  // Complexity: O(1)
  
  Node * node = create_node(datum);
  
  if (list -> head == NULL) {
    list -> head = node;
    list -> tail = node;
    list -> elements = 1;
    
    if (list -> doubly_linked) {
      node -> prev = node;
      node -> next = node;
    }
    return;
  }

  list -> elements++;
  
  if (!list -> doubly_linked) {
    node -> next = list -> head;
    list -> head = node;
    return;
  }
  
  // list is a DLL
  
  // connect before insertion
  node -> next = list -> head;
  node -> prev = list -> head -> prev;

  // make node the new head
  list -> head -> prev -> next = node;
  list -> head -> prev = node;
  list -> head = node;
}

void list_insert_last(List * list, void * datum) {
  // Inserts new node after the tail
  // Currently requires the list be an SLL!
  
  Node * node = create_node(datum);
  
  if (!list -> head) {
    list -> head = node;          // First element, so set
    list -> tail = node;          // -- head and tail here
    list -> elements = 1;
    return;
  }

  list -> tail -> next = node;
  list -> tail = node;
  list -> elements++;
}

void list_remove(List * list, Node * node) {
  // Removes node from list
  // Complexity: O(n) for SLLs
  // Complexity: O(1) for DLLs
  
  if (!list -> doubly_linked) {
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
  if (list -> free) list -> free(node -> value);
  free(node);
}

void list_empty(List * list) {
  // Removes all elements in the list
  
  while (list -> elements) {
    list_remove(list, list -> head);
  }
}

void list_destroy(List * list) {
  // Completely destroys the list
  
  list_empty(list);
  free(list);
}
