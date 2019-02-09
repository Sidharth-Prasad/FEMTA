#ifndef HEADER_GAURD_LIST
#define HEADER_GAURD_LIST

#include <stdbool.h>

#include "types.h"

#define SLL 0
#define DLL 1

typedef struct Node {
  
  struct Node * next;
  struct Node * prev;
  
  void * value;
  
} Node;

typedef struct List {
  
  Node * head;              // The head of the list (has a value)
  Node * tail;              // The tail of the list (has a value) [Only SLL!]
  uint elements;            // Number of elements in the list
  
  bool doubly_linked;       // SLL or DLL (DLL ring if elements limit)
  lambda free;              // how to free elements
  
} List;


Node * create_node(void * value);
List * create_list(bool doubly_linked, lambda freer);

List * create_list_from(bool doubly_linked, lambda freer, uint vargs, ...);

void list_insert(List * list, void * datum);
void list_remove(List * list, Node * node);

void list_insert_last(List * list, void * datum);

void list_empty(List * list);
void list_destroy(List * list);

#endif
