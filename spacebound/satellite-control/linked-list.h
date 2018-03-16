#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_LINKED_LIST
#define HEADER_GAURD_FOR_COMPILER_UNIT_LINKED_LIST


typedef struct Node Node;
typedef struct Node {

  Node * next;
  Node * prev;

  int ivalue;
  float fvalue;
  
} Node;

typedef struct List {

  Node * head;
  unsigned int number_of_elements;
  unsigned int number_of_elements_limit;
  
} List;

Node * create_inode(int value);
Node * create_fnode(float value);
List * create_list(unsigned int limit);
void list_insert(List * list, Node * node);

#endif
