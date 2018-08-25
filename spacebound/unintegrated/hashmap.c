

#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "math.h"
#include "linked-list.h"
#include "hashmap.h"

// Note: you may notice some functions would be shorter and less repetative
// if written via calls to the other functions. While I would usually do this,
// hashing is expensive, so I've done that as few times as possible.


void * hashmap_get   (Hashmap * this, char * string);
void   hashmap_add   (Hashmap * this, char * string, void * datum);
void   hashmap_update(Hashmap * this, char * string, void * datum);
bool   hashmap_exists(Hashmap * this, char * string);
void   hashmap_remove(Hashmap * this, char * string);

Hashmap * create_hashmap(int expected_size) {
  // Creates a hashmap with starting size as a function of expectations

  Hashmap * map = malloc(sizeof(Hashmap));
  
  map -> elements = 0;
  map -> size = expected_size * 2;

  map -> table = malloc(map -> size * sizeof(List **));

  // Create table of singlely linked lists
  for (int i = 0; i < map -> size; i++) {
    map -> table[i] = create_list(0, true);
  }

  // Connect function pointers
  map -> get    = hashmap_get;
  map -> add    = hashmap_add;
  map -> exists = hashmap_exists;
  map -> remove = hashmap_remove;
  map -> update = hashmap_update;

  return map;
}

int hash(char * string, int upper_bound) {
  // Hashes a string into a range using java's algorithm
  // h(x) has been chosen to minimize collisions
  
  int hx = 0;
  for (int i = 0; i < strlen(string); i++) {
    hx = hx * 31 + string[i];
  }
  
  return abs(hx) % upper_bound;
}

void * hashmap_get(Hashmap * this, char * string) {
  // Gets element from hashmap

  int hx = hash(string, this -> size);

  List * list = this -> table[hx];

  // Ensure list isn't empty
  if (!this -> table[hx] -> elements) {
    printf("Tried to retrieve element that does not exist\n");
    exit(1);
  }
  
  Node * node = (Node *) 0x1;   // Used to bypass first loop condition

  for (; node && node != list -> head; node = node -> next) {

    if (node == (Node *) 0x1) node = list -> head;   // bypass complete

    char * key = ((HashmapElement *) node -> value) -> key;
    
    if (strcmp(string, key)) continue;

    // string matches key
    return ((HashmapElement *) node -> value) -> datum;
  }

  // No match was found in the matching table list
  printf("Tried to retrieve element that does not exist\n");
  exit(1);
}

void hashmap_add(Hashmap * this, char * string, void * datum) {
  // Adds an element to the hashmap, doubling its size if necessary

  int hx = hash(string, this -> size);

  HashmapElement * element = malloc(sizeof(HashmapElement));
  element -> key = string;
  element -> datum = datum;

  list_insert(this -> table[hx], create_node(element));
  
}

bool hashmap_exists(Hashmap * this, char * string) {
  return false;
  //return this -> valid[hash(string, this -> size)];
}

void hashmap_remove(Hashmap * this, char * string) {
  // Removes element, making sure it exists

  int hx = hash(string, this -> size);

  List * list = this -> table[hx];

  Node * node = (Node *) 0x1;   // Used to bypass first loop condition

  for (; node && node != list -> head; node = node -> next) {

    if (node == (Node *) 0x1) node = list -> head;   // bypass complete

    char * key = ((HashmapElement *) node -> value) -> key;
    
    if (strcmp(string, key)) continue;

    // string matches key
    list_remove(list, node);
    return;
  }

  // No match was found in the matching table list
  printf("Tried to remove element that does not exist\n");
  exit(1);
}

void hashmap_update(Hashmap * this, char * string, void * datum) {
  // Updates an element in the hashmap
  // Note this does not free the old value,
  // if freeing is necessary, do so by freeing via free(get());
  // However, I intend to use this to store indices, which are ints.

  int hx = hash(string, this -> size);

  List * list = this -> table[hx];

  Node * node = (Node *) 0x1;   // Used to bypass first loop condition

  for (; node && node != list -> head; node = node -> next) {

    if (node == (Node *) 0x1) node = list -> head;   // bypass complete
    
    char * key = ((HashmapElement *) node -> value) -> key;
    
    if (strcmp(string, key)) continue;

    // string matches key
    ((HashmapElement *) node -> value) -> datum = datum;
    return;
  }

  // No match was found in the matching table list
  printf("Tried to update an element that does not exist\n");
  exit(1);
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

int main() {

  Hashmap * map = create_hashmap(4);

  printf("\nAdding 8 elements\n");
  map ->    add(map, "humanity", (void *)   0);
  map ->    add(map, "universe", (void *)  64);
  map ->    add(map, "unity"   , (void *)  32);
  map ->    add(map, "computer", (void *) 256);
  map ->    add(map, "abacus"  , (void *)   8);
  map ->    add(map, "calculx" , (void *)  16);
  map ->    add(map, "aero"    , (void *) 128);
  map ->    add(map, "mayday"  , (void *) 128);
  hashmap_print(map);

  printf("updating 2 elements\n");
  map -> update(map, "mayday"  , (void *)  32);
  map -> update(map, "mayday"  , (void *)  16);
  hashmap_print(map);
  
  printf("removing 2 elements\n");
  map -> remove(map, "aero"                  );
  map -> remove(map, "abacus"                );
  
  printf("updating 2 elements\n");
  map -> update(map, "unity"   , (void *)  64);
  map -> update(map, "calculx" , (void *)  32);
  
  printf("humanity: %d\n", (int) map -> get(map, "humanity"));
  printf("universe: %d\n", (int) map -> get(map, "universe"));
  //printf("abacus: %d\n"  , (int) map -> get(map, "abacus"  ));
  //printf("aero: %d\n"    , (int) map -> get(map, "aero"    ));
  
  hashmap_print(map);
  
  return 0;
}

