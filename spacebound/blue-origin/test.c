
#include <stdlib.h>
#include <stdio.h>

#include "list.h"

void print_list(List * list) {

  for (Node * node = list -> head; node; node = node -> next) {
    printf("%s\t", node -> value);
  }
  printf("\n");

  for (Node * node = list -> head; node; node = node -> next) {
    printf("%p\t", node -> next);
  }
  printf("\n");

}

int main() {

  List * list = create_list(SLL, free);

  list_insert(list, "humanity");
  list_insert(list, "programming");
  list_insert(list, "blank");
  
  print_list(list);
}
