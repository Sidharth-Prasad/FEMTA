

#include <stdlib.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

#include "i2c.h"

#define I2C_ADDR 0x53

i2c_device * create_i2c_device(char * name, uchar8 address) {
  
  i2c_device * i2c = malloc(sizeof(i2c_device));
  
  
  
  return i2c;
}

int main() {
  
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

  char buffer[1] = {
    0x00,
  };
  write(fd, buffer, 1);
  
  read(fd, buffer, 1);
  
  printf("0x%02X\n", buffer[0]);
  return 0;
}
