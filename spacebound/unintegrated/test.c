
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "linked-list.h"
#include "hashmap.h"
#include "colors.h"
#include "error.h"
#include "scripter.h"

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

int test_list() {

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
  

  if (tests_passed == tests) printf(CONSOLE_GREEN "passed all %d list tests\n" CONSOLE_RESET, tests);
  else printf(CONSOLE_RED "passed %d out of %d tests\n" CONSOLE_RESET, tests_passed, tests); 
  
  return (tests_passed == tests);
}

void hashmap_print(Hashmap * map) {

  printf("printing out hashmap\n");
  
  for (int i = 0; i < map -> size; i++) {

    printf("list %d\n", i);

    // Ensure list isn't empty
    if (!map -> table[i] -> elements) continue;

    Node * node = (Node *) 0x1;   // Used to bypass first loop condition

    for (; node != map -> table[i] -> head; node = node -> next) {

      if (node == (Node *) 0x1) node = map -> table[i] -> head;
      
      HashmapElement * element = (HashmapElement *) node -> value;
      
      printf("\t(%s, %d)\n", element -> key, (int) element -> datum);
      
    }
  }

  printf("\n");
}

int test_hashmap() {

  printf("\nTesting hashmap\n");
  
  Hashmap * map = create_hashmap(4);

  printf("Adding 8 elements\n");
  map ->    add(map, "humanity", (void *)   0);
  map ->    add(map, "universe", (void *)  64);
  map ->    add(map, "unity"   , (void *)  32);
  map ->    add(map, "computer", (void *) 256);
  map ->    add(map, "abacus"  , (void *)   8);
  map ->    add(map, "calculx" , (void *)  16);
  map ->    add(map, "aero"    , (void *) 128);
  map ->    add(map, "mayday"  , (void *) 128);
  //hashmap_print(map);

  printf("updating 2 elements\n");
  map -> update(map, "mayday"  , (void *)  32);
  map -> update(map, "mayday"  , (void *)  16);
  //hashmap_print(map);
  
  printf("removing 2 elements\n");
  map -> remove(map, "aero"                  );
  map -> remove(map, "abacus"                );
  
  printf("updating 2 elements\n");
  map -> update(map, "unity"   , (void *)  64);
  map -> update(map, "calculx" , (void *)  32);

  hashmap_print(map);
  
  int tests = 2;
  int tests_passed = 2;
  
  int expected_elements[8] = {0, 2, 0, 1, 1, 2, 0, 0};
  int expected_values[6] = {16, 0, 64, 32, 256, 64};

  if (map -> elements != 6) tests_passed--;   // Ensure proper number of elements recorded
  else {
    // Make sure element count in each list is as expected
    
    for (int i = 0; i < map -> size; i++) {
      if (map -> table[i] -> elements != expected_elements[i]) {
        tests_passed--;
        break;
      }
    }
  }
  if (tests_passed == 2) printf(CONSOLE_GREEN "PASSED: Hashmap location test\n" CONSOLE_RESET);
  else printf(CONSOLE_RED "FAILED: Hashmap location test\n" CONSOLE_RESET);
  
  int expectation_index = 0;
  for (int i = 0; i < map -> size; i++) {

    if (!map -> table[i] -> elements) continue;   // Ensure list isn't empty

    Node * node = (Node *) 0x1;   // Used to bypass first loop condition

    for (; node != map -> table[i] -> head; node = node -> next) {

      if (node == (Node *) 0x1) node = map -> table[i] -> head;
      
      HashmapElement * element = (HashmapElement *) node -> value;

      if ((int) element -> datum != expected_values[expectation_index++]) {
        printf(CONSOLE_RED "FAILED: Hashmap location test\n\n" CONSOLE_RESET);
        tests_passed--;
        break;
      }
    }

    if (!tests_passed) break;
  }
  
  if (tests_passed == 2) {
    printf(CONSOLE_GREEN "PASSED: Hashmap value test\n\n" CONSOLE_RESET);
    printf(CONSOLE_GREEN "passed all %d hashmap tests\n" CONSOLE_RESET, tests);
  }
  else printf(CONSOLE_RED "PASSED: %d out of %d hashmap tests\n" CONSOLE_RESET, tests_passed, tests);
  
  //printf("humanity: %d\n", (int) map -> get(map, "humanity"));
  //printf("universe: %d\n", (int) map -> get(map, "universe"));
  //printf("abacus: %d\n"  , (int) map -> get(map, "abacus"  ));
  //printf("aero: %d\n"    , (int) map -> get(map, "aero"    ));
  
  return tests_passed;
}

// Compiler promises for each function
void print_string(void * string);
void double_integer(void * integer);
void random_pointer(void * nil);
void random_float(void * nil);

int test_scripter() {

  initialize_scripter();
  
  define_script_action("print"         , (lambda) print_string  );
  define_script_action("double"        , (lambda) double_integer);
  define_script_action("random_pointer", (lambda) random_pointer);
  define_script_action("random_decimal", (lambda) random_float  );

  printf("\nTesting action table entries\n");
  
  void * a = action_table -> get(action_table, "print"         );
  void * b = action_table -> get(action_table, "double"        );
  void * c = action_table -> get(action_table, "random_pointer");
  void * d = action_table -> get(action_table, "random_decimal");
  
  printf("\t%p\t%p\n", a, print_string  );
  printf("\t%p\t%p\n", b, double_integer);
  printf("\t%p\t%p\n", c, random_pointer);
  printf("\t%p\t%p\n", d, random_float  );

  char tests_failed = 0;
  
  if (a == print_string && b == double_integer && c == random_pointer && d == random_float) {
    printf(CONSOLE_GREEN "SUCCESS: action table test\n\n" CONSOLE_RESET);
  }
  else {
    printf(CONSOLE_RED "FAILURE: action table test\n\n" CONSOLE_RESET);
    tests_failed++;
  }
  
  execute_script("scripts/test.txt");

  hashmap_print(action_table);
  
  return tests_failed;
}


// Functions for testing

void print_string(void * string) {
  printf(CONSOLE_YELLOW "\t-> %s" CONSOLE_RESET, (char *) string);
}

void double_integer(void * integer) {
  printf(CONSOLE_YELLOW "\t-> %d\n" CONSOLE_RESET, 2 * atoi((char *) integer));
}

void random_pointer(void * nil) {
  printf(CONSOLE_YELLOW "\t-> %p\n" CONSOLE_RESET, (void *) rand());
}

void random_float(void * nil) {
  printf(CONSOLE_YELLOW "\t-> %f\n" CONSOLE_RESET, (((float) rand()) / 4294967296));
}



int main () {
  return test_list() + test_hashmap() + test_scripter();
}
