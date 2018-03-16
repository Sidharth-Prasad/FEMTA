
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "linked-list.h"

Node * create_inode(int value) {
  Node * node = malloc(sizeof(Node));
  node -> ivalue = value;
  return node;
}

Node * create_fnode(float value) {
  Node * node = malloc(sizeof(Node));
  node -> fvalue = value;
  return node;
}

// Creates a list with potentially limited length
List * create_list(unsigned int limit) {
  List * list = malloc(sizeof(List));
  list -> head = NULL;
  list -> number_of_elements = 0;
  list -> number_of_elements_limit = limit;
  return list;
}

// Note : Node may be destroyed
void list_insert(List * list, Node * node) {
  if (list -> head == NULL) {
    list -> head = node;
    list -> number_of_elements = 1;
    node -> prev = node;
    node -> next = node;
    return;
  }

  // Limit does not exist or has not been reached
  if (!list -> number_of_elements_limit || list -> number_of_elements < list -> number_of_elements_limit) {
    node -> next = list -> head;
    node -> prev = list -> head -> prev;
    list -> head -> prev -> next = node;
    list -> head -> prev = node;
    list -> head = node;
    list -> number_of_elements++;
    return;
  }

  // Limit has been reached
  list -> head -> prev -> ivalue = node -> ivalue;
  list -> head -> prev -> fvalue = node -> fvalue;
  list -> head = list -> head -> prev;
  free(node);
}

/*
void print_list(List * list) {
  printf("%d\t", list -> head -> ivalue);
  for (Node * node = list -> head -> next; node != list -> head; node = node -> next) {
    printf("%d\t", node -> ivalue);
  }
  printf("\n");
}

int main() {
  
  List * list = create_list(4);
  //print_list(list);

  for (int i = 0; i < 8; i++) {
    list_insert(list, create_inode(i));
    print_list(list);
  }

}
*/
