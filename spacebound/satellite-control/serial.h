#ifndef HEADER_GAURD_SERIAL_INTERFACE
#define HEADER_GAURD_SERIAL_INTERFACE

#include <stdbool.h>
#include <stdint.h>

#include "graphics.h"
#include "logger.h"

typedef signed char schar;


typedef struct Serial {

  schar handle;
  

} Serial;

typedef struct Packet {
  uint8_t  address;
  uint8_t  type;
  uint16_t checksum;
  uint8_t  data_length;
  uint8_t  data[32];
} Packet;


bool initialize_serial();
void terminate_serial();

long serial_delay;

Logger * serial_logger;

#endif
