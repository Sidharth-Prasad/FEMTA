#ifndef HEADER_GAURD_LINKED_LIST
#define HEADER_GAURD_LINKED_LIST

#include <stdbool.h>

#include "types.h"

typedef struct Node Node;
typedef struct List List;

typedef double (* Gate)(List *)

typedef struct Node {
  
  double probability;
  
  List * children;

  Node * parent;
  
} Node;

typedef struct List {

  Node * head;                    // The head of the list (has a value)

  Gate gate;
  
  unsigned int elements;          // Number of elements in the list
  
} List;


List * create_list();

List * create_list_from(uint vargs, ...);

void list_insert(List * list, Node * node);

#endif
