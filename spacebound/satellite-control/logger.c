
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "logger.h"

// Compiler promises
bool open_prototype    (Logger * self);
bool close_prototype   (Logger * self);
void destroy_prototype (Logger * self);


// Creates a logger, tying the function pointers
Logger * create_logger(char * filename) {
  Logger * logger = malloc(sizeof(Logger));
  logger -> filename = filename;
  logger -> values_read = 0;
  
  // Connect function pointers
  logger -> open    = &open_prototype;
  logger -> close   = &close_prototype;
  logger -> destroy = &destroy_prototype;
  return logger;
}


/***************************** Prototypes *****************************
 *
 * The following functions are tied to logger instance structures
 * Do not attempt to call these outside of the class.
 * These are prototypes in that they help build this pseudo-class,
 * not in the sense that they are incomplete or need revising.
 *
 **********************************************************************/

bool open_prototype(Logger * self) {

  // Open the file in 'append' mode
  self -> file = fopen(self -> filename, "a");
}

bool close_prototype(Logger * self) {

  // Close the file
  fclose(self -> file);
  self -> file = NULL;
}

void destroy_prototype(Logger * self) {
  
  // Make functions uncallable
  self -> open    = NULL;
  self -> close   = NULL;
  self -> destroy = NULL;

  // Free structure
  free(self -> file);
  free(self        );
}
