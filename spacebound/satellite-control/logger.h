#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_LOGGER
#define HEADER_GAURD_FOR_COMPILER_UNIT_LOGGER

#include <stdio.h>
#include <stdlib.h>

typedef struct Logger Logger;
typedef struct Logger {
  Logger * self;
  FILE * file;
  char * filename;
  pthread_t thread;
  bool termination_signal;       // used to terminate child thread
  int values_read;
  float gx, gy, gz, ax, ay, az, mx, my, mz; //sorry testing this out
  
  bool (*open   )(Logger * self);
  bool (*close  )(Logger * self);
  void (*destroy)(Logger * self);
} Logger;

Logger * create_logger(char * log_file_name);

#endif
