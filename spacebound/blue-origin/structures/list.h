#ifndef HEADER_GAURD_LIST
#define HEADER_GAURD_LIST

#include <stdbool.h>

#include "../types/thread-types.h"

typedef void (* ValueFree)(void *);

typedef struct ListNode {
  
  struct ListNode * next;
  struct ListNode * prev;
  
  void * value;
  
} ListNode;

typedef struct List {
  
  ListNode * head;       // the head of the list (has a value)
  ValueFree free;        // means by which nodes are free
  
  int size;              // number of elements in the list

  Mutex * lock;          // lock for when list is parallel
  
} List;


List * list_create();                                 // creates an empty list
List * list_from(int vargs, ...);                     // creates a list pre-filled with elements
List * list_that_frees(ValueFree freer);              // creates an empty list that frees

void list_insert(List * list, void * datum);          // inserts a node at the end of the list
void list_insert_first(List * list, void * datum);    // inserts a node at the beginning of the list
void list_remove(List * list, ListNode * node);       // removes a node
void list_concat(List * first, List * other);         // appends the second list to the end of the first
void list_lock(List * list);                          // locks a parallel list
void list_unlock(List * list);                        // unlocks a parallel list

void list_empty(List * list);                         // removes all nodes in the list
void list_destroy(List * list);                       // removes all nodes and frees the list itself


#define iterate(LIST, TYPE, NAME)                                           \
  TYPE                                                                      \
  NAME             = (TYPE) *(int  *) &(LIST -> head),                      \
    * NAME##_node  =         (void *)  (LIST -> head),                      \
    * NAME##_index = 0;                                                     \
                                                                            \
  (int) NAME##_index < (LIST -> size) &&				    \
  ((NAME = (TYPE) *(int *) &((ListNode *) NAME##_node) -> value) || true);  \
                                                                            \
  NAME##_index   = ((void *) NAME##_index + 1),                             \
    NAME##_node  =  (void *) ((ListNode *) NAME##_node) -> next,            \
    NAME         = (TYPE) *(int *) &(NAME##_node)

#endif
