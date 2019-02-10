

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <pigpio.h>


#include "i2c.h"
#include "sensor.h"
#include "adxl.h"
#include "color.h"
#include "clock.h"
#include "list.h"

void * i2c_main();

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint8 interval) {
  // creates an i2c device, adding it to the device list
  
  i2c_device * i2c = malloc(sizeof(i2c_device));
  
  i2c -> sensor   = sensor;
  i2c -> read     = reader;
  i2c -> interval = interval;
  
  i2c -> count = 0;
  
  i2c -> handle = i2cOpen(1, address, 0);
  
  printf("Added i2c device %d\n", i2c -> handle);
  
  list_insert(schedule -> devices, i2c);
  
  return i2c;
}

void i2c_freer(void * device_ptr) {
  // closes and frees the i2c device
  
  i2c_device * i2c = (i2c_device *) device_ptr;
  
  i2cClose(i2c -> handle);
  free(i2c);
}

void init_i2c() {
  //
  
  schedule = malloc(sizeof(i2c_schedule));
  
  schedule -> devices = create_list(SLL, i2c_freer);
  schedule -> thread  = malloc(sizeof(pthread));
  
  // open communication with the i2c bus
  
  
}

void start_i2c() {
  
  // create i2c thread
  if (pthread_create(schedule -> thread, NULL, i2c_main, NULL)) {
    printf(CONSOLE_RED "Could not start i2c thread\n" CONSOLE_RESET);
    return;  
  }
}


void i2c_read_bytes(int handle, uint8 reg, uint8 * buf, char n) {
  
  if (i2cReadI2CBlockData(handle, reg, buf, n) < 0) {
    printf(CONSOLE_RED "Could not read bytes\n" CONSOLE_RESET);
    exit(3);
  }
}

void i2c_write_byte(int handle, uint8 reg, uint8 value) {
  // writes a byte to the device associated with the handle

  if (i2cWriteByteData(handle, reg, value) < 0) {
    printf(CONSOLE_RED "Could not write byte\n" CONSOLE_RESET);
    exit(3);
  }
  
}


void * i2c_main() {
  
  while (!schedule -> term_signal) {
    
    for (Node * node = schedule -> devices -> head; node; node = node -> next) {
      
      i2c_device * i2c = (i2c_device *) node -> value;
      
      i2c -> count += 10;
      
      if (i2c -> count == i2c -> interval) {
        
        (i2c -> read)(i2c);
        
        i2c -> count = 0;
      }
    }
    
    real_milli_sleep(10);    // 10ms
  }

}


void terminate_i2c() {
  // frees everything associated with the i2c system
  
  list_destroy(schedule -> devices);      // note that this kills
  
  free(schedule -> thread);
  free(schedule);
}
