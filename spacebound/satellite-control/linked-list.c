
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "linked-list.h"
#include "string.h"
#include "colors.h"

Node * create_node(void * value) {
  Node * node = malloc(sizeof(Node));
  node -> value = value;
  node -> next = NULL;
  node -> prev = NULL;
  return node;
}

// Creates a list with potentially limited length
List * create_list(unsigned int limit, bool doublely_linked) {
  List * list = malloc(sizeof(List));
  list -> head = NULL;
  list -> elements = 0;
  list -> elements_limit = limit;
  list -> doublely_linked = doublely_linked;
  return list;
}

void list_insert(List * list, Node * node) {
  // Inserts node into the linked list
  // Complexity: O(1)

  if (list -> head == NULL) {
    list -> head = node;
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
  }

  else {
    node -> next -> prev = node -> prev;   // Drop out of DLL
    node -> prev -> next = node -> next;   // ---------------
  }

  if (node == list -> head) {
    list -> head = node -> next;
  }

  if (--list -> elements == 0) list -> head = NULL;
  free(node);

  
}
