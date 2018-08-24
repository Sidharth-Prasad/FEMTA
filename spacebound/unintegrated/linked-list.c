
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "linked-list.h"
#include "string.h"
#include "colors.h"

Node * create_node(void * value) {
  Node * node = malloc(sizeof(Node));
  node -> value = value;
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
  // Note: Currently only does anything for DLLs
  // Note: I have not considered limited DLLs
  
  if (list -> doublely_linked) {

    list -> elements--;
    
    if (node == list -> head) {
      list -> head = node -> next;
      free(node);
      return;
    }
    
    if (node -> next = NULL) {
      free(node);
      return
    }

    node -> next -> prev = node -> prev;
    node -> prev -> next = node -> next;
    free(node);
  }
  
}

