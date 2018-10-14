#ifndef HEADER_GAURD_LINKED_LIST
#define HEADER_GAURD_LINKED_LIST

#include <stdbool.h>

#define INTEGER_NODE 0
#define FLOAT_NODE   1
#define STRING_NODE  2

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

  Node * head;
  unsigned int elements;
  unsigned int elements_limit;

  bool doublely_linked;
  bool auto_free;                // Do not use with element limits!
  
} List;

Node * create_node(void * value);
List * create_list(unsigned int limit, bool doublely_linked, bool auto_free);
void list_insert(List * list, Node * node);
void list_remove(List * list, Node * node);

#endif
