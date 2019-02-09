

#include <stdlib.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
//#include <linux/i2c.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

#include "i2c.h"
#include "sensor.h"
#include "adxl.h"
#include "color.h"
#include "clock.h"

void * i2c_main();

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint8 interval) {
  // creates an i2c device, adding it to the device list
  
  i2c_device * i2c = malloc(sizeof(i2c_device));
  
  i2c -> sensor   = sensor;
  i2c -> read     = reader;
  i2c -> interval = interval;
  
  i2c -> count = 0;
  
  list_insert(schedule -> devices, i2c);
  
  return i2c;
}

void init_i2c() {
  //
  
  schedule = malloc(sizeof(i2c_schedule));
  
  schedule -> devices = create_list(SLL, NULL);
  schedule -> thread  = malloc(sizeof(pthread));
  
  // open communication with the i2c bus
  
  if ((schedule -> fd = open("/dev/i2c-1/", O_RDWR)) < 0) {
    printf(CONSOLE_RED "Failed to establish i2c communication\n" CONSOLE_RESET);
    exit(2);
  }
  
  schedule -> last_addr = 0xFF;    // set last address used to something impossible (0xFF > 0x7F)
}

void start_i2c() {
  
  // create i2c thread
  if (pthread_create(schedule -> thread, NULL, i2c_main, NULL)) {
    printf(CONSOLE_RED "Could not start i2c!\n" CONSOLE_RESET);
    return;  
  }
}

void assert_address(uint8 address) {

  // see if we've already asserted this address
  if (schedule -> last_addr == address) return;
  
  // need to assert different address
  if (ioctl(schedule -> fd, I2C_SLAVE, address) < 0) {
    printf(CONSOLE_RED "Could not assert address %u\n" CONSOLE_RESET, address);
    exit(2);
  }
}

void i2c_read_bytes(uint8 address, uint8 reg, int n) {
  
  assert_address(address);
  
  i2c_smbus_read_byte_data(schedule -> fd, reg);
  
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


/*int main() {
  
  int fd = open("/dev/i2c-1", O_RDWR);
  
  if (fd < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  
  if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    return 1;
  }
  
  /*buffer[0]=0xFF;
    write(fd, buffer, 1);*/

/*char buffer[1] = {
    0x00,
  };
  write(fd, buffer, 1);
  
  read(fd, buffer, 1);
  
  printf("0x%02X\n", buffer[0]);
  return 0;
}
*/
