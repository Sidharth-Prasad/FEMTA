#ifndef HEADER_GAURD_LINKED_LIST
#define HEADER_GAURD_LINKED_LIST

#include <stdbool.h>

#include "types.h"

typedef struct Node Node;
typedef struct Node {

  union {
    Node * next;
    Node * right;
  };
  
  union {
    Node * prev;
    Node * left;
    Node * child;
  };
  
  void * value;
  
} Node;

typedef struct List {

  Node * head;                    // The head of the list (has a value)
  Node * tail;                    // The tail of the list (has a value) [Only SLL!]
  unsigned int elements;          // Number of elements in the list
  unsigned int elements_limit;    // Turns list into DLL Ring if not 0

  bool doublely_linked;           // SLL or DLL (DLL ring if elements limit)
  bool auto_free;                 // Do not use with element limits!
  
} List;


Node * create_node(void * value);
List * create_list(uint limit, bool doublely_linked, bool auto_free);

List * create_list_from(uint vargs, ...);

void list_insert(List * list, Node * node);
void list_remove(List * list, Node * node);

void list_insert_last(List * list, void * datum);

void list_destroy(List * list);

#endif
