
#include "stdlib.h"
#include "stdio.h"
#include "hashmap.h"
#include "scripter.h"



void initialize_scripter() {

  action_table = create_hashmap(16);
  
  
}

void define_script_action(char * symbol, lambda action) {
  
  
  
}


void   print_string(char * string);
int    add_numbers(int a, int b);
void * random_pointer();
float  random_float();

int main() {

  define_script_action("print"         , (lambda) print_string);
  define_script_action("add"           , (lambda) print_string);
  define_script_action("random_pointer", (lambda) print_string);
  define_script_action("random_decimal", (lambda) print_string);
  
  return 0;
}

void print_string(char * string) {
  printf("%s\n", string);
}

int add_numbers(int a, int b) {
  return a + b;
}

void * random_pointer() {
  return (void *) rand();
}

float random_float() {
  return (((float) rand()) / 4294967296);
}
