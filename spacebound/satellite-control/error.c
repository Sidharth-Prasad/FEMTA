
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "colors.h"
#include "error.h"
#include "logger.h"

typedef pthread_t         pthread;
typedef pthread_mutex_t   pmutex;

pmutex * error_mutex;

void init_error_handling() {
  // Create the logger and mutex for error logging
  
  error_mutex = malloc(sizeof(pmutex));

  if (pthread_mutex_init(error_mutex, NULL)) {
    exit_printing("Fatal: unable to create error log mutex");
  }
  
  error_logger = create_logger("./logs/error-log.txt");
  error_logger -> open(error_logger);
}

void exit_printing(char * message, char code) {
  // exit due to deleterious error, printing to the console
  
  printf(CONSOLE_RED "%s\n" CONSOLE_RESET, message);
  exit(code);
}

void log_error(char * message) {

  pthread_mutex_lock(error_mutex);
  
  fprintf(error_logger -> file, message);
  
  pthread_mutex_unlock(error_mutex);
}

void terminate_error_handling() {
  error_logger -> close(error_logger);
  error_logger -> destroy(error_logger);
}
