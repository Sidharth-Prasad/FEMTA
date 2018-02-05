#ifndef HEADER_GAURD_FOR_COMPILER_UNIT_UART_INTERFACE
#define HEADER_GAURD_FOR_COMPILER_UNIT_UART_INTERFACE

typedef struct UART {

  unsigned char UART_address;

  // Read Functions
  float (* temperature)();

} UART;

bool initialize_UART(module * initialent);

module * serial_device;

#endif
