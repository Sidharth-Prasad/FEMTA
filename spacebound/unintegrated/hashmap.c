

#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "math.h"
#include "linked-list.h"
#include "hashmap.h"

void * hashmap_get   (Hashmap * this, char * string);
void   hashmap_add   (Hashmap * this, char * string, void * datum);
bool   hashmap_exists(Hashmap * this, char * string);
void   hashmap_remove(Hashmap * this, char * string);
void   hashmap_update(Hashmap * this, char * string);

Hashmap * create_hashmap(int expected_size) {
  // Creates a hashmap with starting size as a function of expectations

  Hashmap * map = malloc(sizeof(Hashmap));
  
  map -> elements = 0;
  map -> size = (expected_size + 1) * 2;

  map -> table = malloc(map -> size * sizeof(List **));
  map -> valid = malloc(map -> size * sizeof(bool   ));

  // Create table of singlely linked lists
  for (int i = 0; i < map -> size; i++) {
    map -> table[i] = create_list(0, true);
  }                                    
  
  // Clear initial values in hashmap
  for (int i = 0; i < map -> size; i++) {
    *(map -> valid + i) = false;
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
  
  return hx % upper_bound;
}

void * hashmap_get(Hashmap * this, char * string) {
  // Gets element from hashmap

  int hx = hash(string, this -> size);

  // Make sure an element exists
  if (!this -> valid[hx]) {
    printf("Tried to retrieve element that does not exist\n");
    exit(1);
  }

  List * list = this -> table[hx];

  for (Node * node = list -> head; node; node = node -> next) {

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
  // Adds an element to the hashmap

  int hx = hash(string, this -> size);
  
  /*// See if collision is imminent
  if (this -> valid[hx]) {

    if (!strcmp(string, this -> keys[hx])) {
      printf("Tried to add element already in table\n");
      return;
    }
    
    // double hashmap size and rehash
    this -> size *= 2;
    for (
    }*/
}

bool hashmap_exists(Hashmap * this, char * string) {
  return this -> valid[hash(string, this -> size)];
}

void hashmap_remove(Hashmap * this, char * string) {
  // Removes element, making sure it exists

  int hx = hash(string, this -> size);

  // Make sure an element exists
  if (!this -> valid[hx]) {
    printf("Tried to retrieve element that does not exist\n");
    exit(1);
  }

  List * list = this -> table[hx];

  for (Node * node = list -> head; node; node = node -> next) {

    char * key = ((HashmapElement *) node -> value) -> key;
    
    if (strcmp(string, key)) continue;

    // string matches key
    return ((HashmapElement *) node -> value) -> datum;
  }

  // No match was found in the matching table list
  printf("Tried to retrieve element that does not exist\n");
  exit(1);
}

void hashmap_update(Hashmap * this, char * string) {
  
}


int main() {

  Hashmap * map = create_hashmap(64);

  
  return 0;
}

