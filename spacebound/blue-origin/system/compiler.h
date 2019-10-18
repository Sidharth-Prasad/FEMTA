#ifndef HEADER_GAURD_COMPILER
#define HEADER_GAURD_COMPILER

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#endif
