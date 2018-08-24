
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "linked-list.h"
#include "colors.h"

void print_list(List * list, short type) {
  printf("Elements: %d\n", list -> elements);
  printf("Values:\t");
  
  Node * node = (Node *) 0x1;   // Not NULL, used to bypass first for-loop check
  
  for (; node && node != list -> head; node = node -> next) {

    if (node == (Node *) 0x1) node = list -> head;

    if      (type == INTEGER_NODE) printf("%d\t",  (int    )  node -> value);
    else if (type ==   FLOAT_NODE) printf("%f\t", *(float *) &node -> value);
    else if (type ==  STRING_NODE) printf("%s\t",  (char  *)  node -> value);
  }
  
  printf("\n");
}

bool assert_list(List * list, void * array, short type) {

  int index = 0;

  Node * node = (Node *) 0x1;   // Not NULL, used to bypass first for-loop check
    
  for (; node && node != list -> head; node = node -> next) {

    if (node == (Node *) 0x1) node = list -> head;
      
    // Integer test
    if (type == INTEGER_NODE) {
      printf("(%d %d)\t", (int) node -> value, *((int *) array + index));
      if ((int) node -> value != *((int *) array + index)) {
        printf("\n");
        return false;
      }
    }

    // Float test
    else if (type == FLOAT_NODE) {
      printf("(%f %f)\t", *((float *) &(node -> value)), *((float *) array + index));
      if (*((float *) &(node -> value)) != *((float *) array + index)) {
        printf("\n");
        return false;
      }
    }

    // String test
    else if (type == STRING_NODE) {
      printf("(%s %s)\n", (char *) node -> value, *((char **) array + index));
      if (strcmp((char *) node -> value, *((char **) array + index))) {
        printf("\n");
        return false;
      }
    }
      
    index++;
  }
  printf("\n");
  return true;
}

int test() {

  printf("\n");

  int tests = 8;
  int tests_passed = 0;

  // System test
  printf("Performing hardware test\n");
  printf("char:   %d\n", sizeof(char  ));
  printf("short:  %d\n", sizeof(short ));
  printf("int:    %d\n", sizeof(int   ));
  printf("long:   %d\n", sizeof(long  ));
  printf("float:  %d\n", sizeof(float ));
  printf("double: %d\n", sizeof(double));
  printf("void:   %d\n", sizeof(void  ));
  printf("void *: %d\n", sizeof(void *));
  
  if      (sizeof(int   ) != 4) printf(CONSOLE_RED "System not 32 bit!\n\n" CONSOLE_RESET);
  else if (sizeof(float ) != 4) printf(CONSOLE_RED "System not 32 bit!\n\n" CONSOLE_RESET);
  else if (sizeof(void *) != 4) printf(CONSOLE_RED "System not 32 bit!\n\n" CONSOLE_RESET);
  else {
    printf(CONSOLE_GREEN "System appears to be 32 bit\n\n" CONSOLE_RESET);
    tests_passed++;
  }

  
  // SLL integer test
  printf("Testing singlely-linked list: integers\n");
  List * integer_list = create_list(0, false);

  for (int i = 0; i < 8; i++) {
    list_insert(integer_list, create_node((void *) i));
  }
  
  list_remove(integer_list, integer_list -> head -> next -> next                );   // rm 5
  list_remove(integer_list, integer_list -> head                                );   // rm 7
  list_remove(integer_list, integer_list -> head -> next -> next -> next        );   // rm 2
  list_remove(integer_list, integer_list -> head -> next -> next -> next -> next);   // rm 0

  int integer_array[4] = {6, 4, 3, 1};   // What we expect
  
  if (integer_list -> elements != 4) printf(CONSOLE_RED "FAILED: SLL integer test\n\n" CONSOLE_RESET);
  else if (!assert_list(integer_list, (void *) integer_array, INTEGER_NODE)) {
    printf(CONSOLE_RED "FAILED: SLL integer test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: SLL integer test\n\n" CONSOLE_RESET);
    tests_passed++;
  }

  
  // SLL float test
  printf("Testing singlely-linked list: floats\n");
  float float_array[3];
  int float_array_index = 2;
  List * float_list = create_list(0, false);
  for (float f = 1.37; f < 8.4; f *= 2.1) {
    list_insert(float_list, create_node((void *)(*((int *) &f))));
    float_array[float_array_index--] = f;
  }
  
  if (float_list -> elements != 3) printf(CONSOLE_RED "FAILED: SLL float test\n\n" CONSOLE_RESET);
  else if (!assert_list(float_list, (void *) float_array, FLOAT_NODE)) {
    printf(CONSOLE_RED "FAILED: SLL float test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: SLL float test\n\n" CONSOLE_RESET);
    tests_passed++;
  }

  // SLL string test
  printf("Testing singlely-linked list: strings\n");
  char * string_array[4] = {
    "alpha", "beta", "gamma", "delta",
  };
  List * string_list = create_list(0, false);
  list_insert(string_list, create_node((void *) "delta"));
  list_insert(string_list, create_node((void *) "gamma"));
  list_insert(string_list, create_node((void *) "beta" ));
  list_insert(string_list, create_node((void *) "alpha"));
  if (string_list -> elements != 4) printf(CONSOLE_RED "FAILED: SLL string test\n\n" CONSOLE_RESET);
  else if (!assert_list(string_list, (void *) string_array, STRING_NODE)) {
    printf(CONSOLE_RED "FAILED: SLL string test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: SLL string test\n\n" CONSOLE_RESET);
    tests_passed++;
  }
  
  // DLL integer test
  printf("Testing doublely-linked list: integers\n");
  List * double_integer_list = create_list(0, true);
  

  for (int i = 0; i < 8; i++) {
    list_insert(double_integer_list, create_node((void *) i));
  }
  
  list_remove(double_integer_list, double_integer_list -> head -> prev        );   // rm 0
  list_remove(double_integer_list, double_integer_list -> head                );   // rm 7
  list_remove(double_integer_list, double_integer_list -> head -> prev        );   // rm 1
  list_remove(double_integer_list, double_integer_list -> head                );   // rm 6
  list_remove(double_integer_list, double_integer_list -> head -> next -> next);   // rm 3

  int double_integer_array[3] = {5, 4, 2};
  
  if (double_integer_list -> elements != 3) printf(CONSOLE_RED "FAILED: DLL integer test\n\n" CONSOLE_RESET);
  else if (!assert_list(double_integer_list, (void *) double_integer_array, INTEGER_NODE)) {
    printf(CONSOLE_RED "FAILED: DLL integer test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: DLL integer test\n\n" CONSOLE_RESET);
    tests_passed++;
  }

  // DLL float test
  printf("Testing doublely-linked list: floats\n");
  float double_float_array[2];
  int double_float_array_index = 1;
  List * double_float_list = create_list(0, true);
  for (float f = 10.34; f < 23.4; f *= 1.9) {
    list_insert(double_float_list, create_node((void *)(*((int *) &f))));
    double_float_array[double_float_array_index--] = f;
  }
  if (double_float_list -> elements != 2) printf(CONSOLE_RED "FAILED: DLL float test\n\n" CONSOLE_RESET);
  else if (!assert_list(double_float_list, (void *) double_float_array, FLOAT_NODE)) {
    printf(CONSOLE_RED "FAILED: DLL float test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: DLL float test\n\n" CONSOLE_RESET);
    tests_passed++;
  }

  // DLL string test
  printf("Testing doublely-linked list: strings\n\n");
  printf("\t'human: a medium-sized creature prone to great ambition' -Dwarf Fortress\n\n");
  char * double_string_array[8] = {
    "human:", "a", "medium-sized", "creature", "prone", "to", "great", "ambition",
  };
  List * double_string_list = create_list(0, true);
  list_insert(double_string_list, create_node((void *) "ambition"));
  list_insert(double_string_list, create_node((void *) "great"));
  list_insert(double_string_list, create_node((void *) "to" ));
  list_insert(double_string_list, create_node((void *) "prone"));
  list_insert(double_string_list, create_node((void *) "creature"));
  list_insert(double_string_list, create_node((void *) "medium-sized"));
  list_insert(double_string_list, create_node((void *) "a"));
  list_insert(double_string_list, create_node((void *) "human:"));
  if (double_string_list -> elements != 8) printf(CONSOLE_RED "FAILED: DLL string test\n\n" CONSOLE_RESET);
  else if (!assert_list(double_string_list, (void *) double_string_array, STRING_NODE)) {
    printf(CONSOLE_RED "FAILED: DLL string test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: DLL string test\n\n" CONSOLE_RESET);
    tests_passed++;
  }
  
  // DLL integer ring test
  printf("Testing doublely-linked rink list: integers\n");
  List * integer_ring_list = create_list(4, true);
  int integer_ring_array[4] = {7, 6, 5, 4};
  for (int i = 0; i < 8; i++) {
    list_insert(integer_ring_list, create_node((void *) i));
  }
  print_list(integer_ring_list, INTEGER_NODE);
  if (integer_ring_list -> elements != 4) printf(CONSOLE_RED "FAILED: DLL integer ring test\n\n" CONSOLE_RESET);
  else if (!assert_list(integer_ring_list, (void *) integer_ring_array, INTEGER_NODE)) {
    printf(CONSOLE_RED "FAILED: DLL integer ring test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_GREEN "PASSED: DLL integer ring test\n\n" CONSOLE_RESET);
    tests_passed++;
  }
  

  if (tests_passed == tests) printf(CONSOLE_GREEN "passed all %d tests\n", tests);
  else printf(CONSOLE_RED "passed %d out of %d tests\n" CONSOLE_RESET, tests_passed, tests); 
  
  return (tests_passed == tests);
}



int main () {
  return test();
}
