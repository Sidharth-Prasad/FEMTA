#ifndef COMPILER_GAURD_TYPES
#define COMPILER_GAURD_TYPES

#include <pthread.h>

// Integers
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef signed char schar;

// Parallelism
typedef pthread_t       pthread;
typedef pthread_mutex_t pmutex;

// Lambda Calculus
typedef void (* lambda)(void *);
typedef void (* controller)(float angle, float velocity, float time);

#endif
