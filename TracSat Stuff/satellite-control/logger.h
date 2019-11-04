#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_LOGGER
#define HEADER_GAURD_FOR_COMPILER_UNIT_LOGGER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct Logger Logger;
typedef struct Logger {
  Logger * self;
  FILE * file;
  char * filename;
  int values_read;
  
  bool (* open   )(Logger * self);
  bool (* close  )(Logger * self);
  void (* destroy)(Logger * self);
} Logger;

Logger * create_logger(char * log_file_name);

#endif
