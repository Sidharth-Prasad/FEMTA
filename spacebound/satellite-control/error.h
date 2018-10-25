#ifndef HEADER_GAURD_ERROR
#define HEADER_GAURD_ERROR

#include "logger.h"

#define ERROR_PROGRAMMER 1
#define ERROR_OS_FAILURE 2
#define ERROR_LIBRARY_FAILURE 3

void init_error_handling();
void terminate_error_handling();

void exit_printing(char * message, char code);

void log_error(char * message);

Logger * error_logger;

#endif
