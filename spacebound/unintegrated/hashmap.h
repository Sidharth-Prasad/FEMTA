#ifndef HEADER_GAURD_HASHMAP
#define HEADER_GAURD_HASHMAP

#include "stdbool.h"
#include "linked-list.h"

#define HASHMAP_THRESHOLD .6
#define HASHMAP_DEFAULT_SIZE 64

typedef struct HashmapElement {

  char * key;     // The string used to access this value
  void * datum;   // The content this element points to

} HashmapElement;

// Forward promise Hashmap's definition to compiler
typedef struct Hashmap Hashmap;

// Structure for hashmap
typedef struct Hashmap {
  
  unsigned int elements;
  unsigned int size;

  List ** table;   // output values

  void * (*    get)(Hashmap * this, char * string);
  void   (*    add)(Hashmap * this, char * string, void * datum);
  void   (* remove)(Hashmap * this, char * string);
  bool   (* exists)(Hashmap * this, char * string);
  void   (* update)(Hashmap * this, char * string, void * datum);
  
} Hashmap;

Hashmap * create_hashmap();

int hash(char * string, int upper_bound);

#endif
