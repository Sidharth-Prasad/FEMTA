
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "list.h"
#include "hashmap.h"
#include "parmap.h"

#include "../types/thread-types.h"


Parmap * parmap_create(hash_function hash, key_comparator key_diff, element_freer freer, int expected_size) {
  // Creates a parmap with starting size as a function of expectations
  
  Parmap * map = malloc(sizeof(Parmap));
  
  if (expected_size < 16) map -> size = 16;
  else                    map -> size = expected_size;

  map -> pools    = 16;
  
  map -> hash     = hash;
  map -> free     = freer;
  map -> key_diff = key_diff;
  
  map -> table = malloc(map -> size  * sizeof(List **));
  map -> locks = malloc(map -> pools * sizeof(Mutex *));
  
  map -> pool_elements = malloc(map -> pools * sizeof(int));
  
  // Create table of linked lists
  for (int i = 0; i < map -> size; i++)
    map -> table[i] = NULL;
  
  for (int i = 0; i < map -> pools; i++) {
    map -> locks[i] = malloc(sizeof(Mutex));
    map -> pool_elements[i] = 0;
    
    if (pthread_mutex_init(map -> locks[i], NULL)) {
      printf("Could not create mutex\n");
      exit(1);
    }
  }
  
  return map;
}

void * parmap_get(Parmap * this, void * key) {
  // gets element from parmap
  // returns NULL when no element in found
  
  int hx = (this -> hash)(key, this -> size);
  
  List * list   = this -> table[hx];
  Mutex * mutex = this -> locks[hx % this -> pools];
  
  pthread_mutex_lock(mutex);
  
  if (!this -> table[hx]        ) return NULL;
  if (!this -> table[hx] -> size) return NULL;
  
  for (iterate(list, HashmapElement *, element)) {
    
    void * element_key = element -> key;
    
    if (this -> key_diff(key, element_key)) continue;
    
    return element -> value;
  }

  pthread_mutex_unlock(mutex);
  
  return NULL;
}

void parmap_add(Parmap * this, void * key, void * value) {
  // Adds an element to the parmap
  
  int hx = this -> hash(key, this -> size);
  
  HashmapElement * element = malloc(sizeof(HashmapElement));
  element -> key = key;
  element -> value = value;
  
  Mutex * mutex = this -> locks[hx % this -> pools];
  
  pthread_mutex_lock(mutex);
  
  if (!this -> table[hx])
    this -> table[hx] = list_that_frees(this -> free);
  
  list_insert(this -> table[hx], element);
  
  this -> pool_elements[hx % this -> pools]++;

  pthread_mutex_unlock(mutex);
}

bool parmap_exists(Parmap * this, void * key) {
  return parmap_get(this, key) != NULL;
}

void parmap_remove(Parmap * this, void * key) {
  // Removes element, making sure it exists
  // Note, this does not free the old value. If needed, do that yourself
  // Also! The all list is currently not updated!
  
  int hx = this -> hash(key, this -> size);

  List  * list  = this -> table[hx];
  Mutex * mutex = this -> locks[hx % this -> pools];
  
  pthread_mutex_lock(mutex);

  for (iterate(list, HashmapElement *, element)) {

    void * element_key = element -> key;
    
    if (this -> key_diff(key, element_key)) continue;
    
    element_node = (void *) ((ListNode *) element_node) -> prev;    // pull-back
    
    list_remove(list, ((ListNode *) element_node) -> next);      // ---------
    
    this -> pool_elements[hx % this -> pools]--;

    pthread_mutex_unlock(mutex);
    return;
  }
  
  pthread_mutex_unlock(mutex);
  
  // No match was found in the matching table list
  printf("Tried to remove element that does not exist\n");
  exit(1);
}

void parmap_update(Parmap * this, void * key, void * value) {
  // Updates an element in the parmap
  // Note this does not free the old value,
  // if freeing is necessary, do so by freeing via free(get());
  // However, I intend to use this to store indices, which are ints.
  
  int hx = this -> hash(key, this -> size);
  
  List * list = this -> table[hx];
  Mutex * mutex = this -> locks[hx % this -> pools];
  
  pthread_mutex_lock(mutex);
  
  if (!list) {
    pthread_mutex_unlock(mutex);
    printf("Tried to update an element that does not exist\n");
    exit(1);
  }
  
  for (iterate(list, HashmapElement *, element)) {
    
    void * element_key = element -> key;
    
    if (this -> key_diff(key, element_key)) continue;
    
    element -> value = value;
    pthread_mutex_unlock(mutex);
    return;
  }

  pthread_mutex_unlock(mutex);
  
  // No match was found in the matching table list
  printf("Tried to update an element that does not exist\n");
  exit(1);
}

void parmap_destroy(Parmap * this) {

  for (int i = 0; i < this -> size; i++) {
    
    if (this -> table[i]) {
      list_destroy(this -> table[i]);
      this -> table[i] = NULL;
    }
  }
  
  for (int i = 0; i < this -> pools; i++) {
    pthread_mutex_destroy(this -> locks[i]);
    free(this -> locks[i]);
    this -> locks[i] = NULL;
  }
  free(this -> pool_elements);
  free(this -> table);
  free(this -> locks);
  free(this);
}


void print_parmap_long(HashmapElement * element) {
  printf("  %s %ld\n", (char *) element -> key, (long) element -> value);
}

void parmap_print(Parmap * this, element_printer print) {
  // prints each element in the hash map

  printf("Parmap of size %d\n",   this -> size);

  int elements = 0;

  for (int i = 0; i < this -> pools; i++)
    elements += this -> pool_elements[i];
  
  printf("Contains %d elements\n", elements);

  int largest = 0;
  
  for (int i = 0; i < this -> size; i++) {
    
    if (!this -> table[i]) continue;
    
    printf("%d\n", i);
    
    List * list = this -> table[i];
    
    for (iterate(list, HashmapElement *, element))
      print(element);
    
    if (list -> size > largest)
      largest = list -> size;
  }
  
  printf("\nContains %d elements\n", elements);
  printf("Worst list has %d elements\n", largest);
}
