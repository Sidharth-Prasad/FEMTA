#ifndef HEADER_GAURD_SERIAL_INTERFACE
#define HEADER_GAURD_SERIAL_INTERFACE

#include <stdbool.h>
#include <stdint.h>

#include "graphics.h"
#include "logger.h"
#include "types.h"


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

void null_controller(float angle, float velocity, float time);

long serial_delay;

Logger * serial_logger;
Logger * UM7_euler_logger;
Logger * UM7_vector_logger;
Logger * UM7_quaternion_logger;

controller serial_routine;

#endif
