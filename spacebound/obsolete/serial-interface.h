#ifndef HEADER_GAURD_UART_INTERFACE
#define HEADER_GAURD_UART_INTERFACE

#include "graphics.h"
#include "logger.h"

typedef struct UART {

  signed char serial_handle;

  // Read Functions
  float (* temperature)();

} UART;

typedef struct module module;   // compiler promise

bool initialize_UART(module * initialent);
void terminate_bno_logging();

module * serial_device;

Plot * bno_gyro_plot;
Plot * bno_acel_plot;
Plot * bno_lina_plot;
Plot * bno_magn_plot;

Logger * bno_logger;

#endif
