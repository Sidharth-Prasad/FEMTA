#pragma once

#include <stdbool.h>

#include "list.h"
#include "hashmap.h"

#include "../types/thread-types.h"

typedef struct Parmap {
  
  int size;                   // number of rows in the table
  int pools;                  // number of locks in the table
  
  List ** table;              // output values
  Mutex ** locks;             // locks to control access
  int * pool_elements;
  
  element_freer free;         // means of freeing parmap element
  hash_function hash;         // how hash table indecies are calculated
  key_comparator key_diff;    // how to tell if keys match
  
} Parmap;

Parmap * parmap_create(hash_function hash, key_comparator key_diff, element_freer freer, int expected_size);

void * parmap_get    (Parmap * this, void * key);
void   parmap_add    (Parmap * this, void * key, void * value);
void   parmap_update (Parmap * this, void * key, void * value);
bool   parmap_exists (Parmap * this, void * key);
void   parmap_remove (Parmap * this, void * key);
void   parmap_print  (Parmap * this, element_printer print);
void   parmap_destroy(Parmap * this);

