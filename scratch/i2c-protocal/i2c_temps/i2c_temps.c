#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>

#define MAX31785_TEMP_REG 0x8D
#define MAX31785_TEMP0 6
#define MAX31785_TEMP_INT 12
#define PAGE_REG_OFFSET 6  // Offset in CmdBuf of the page register write value


main( int argc, char *argv[])
{
  int  rcnt;
  char ReadBuf[256];
  char CmdBuf[] = {4, 0x52,  // Chip address
		   2, 7, 2, 0x00, MAX31785_TEMP0, 3,    // Write page register to select temperature sensor
		   2, 7, 1, MAX31785_TEMP_REG, 2, 6, 2, 3, // Read temperature register
		   0 // EOL
  };

  if (gpioInitialise() < 0) return 1;

  // Open bit banging I2C on standard I2C pins
  if (bbI2COpen(2, 3, 100000)) return 1;

  while(1)
    {
      // Loop over the 7 temp sensors
      for(CmdBuf[PAGE_REG_OFFSET] = MAX31785_TEMP0; CmdBuf[PAGE_REG_OFFSET] <= MAX31785_TEMP_INT; CmdBuf[PAGE_REG_OFFSET]++)
	{
	  // Read the temp reg for the current page
	  rcnt = bbI2CZip(2, CmdBuf, sizeof(CmdBuf), ReadBuf, sizeof(ReadBuf));

	  if(rcnt == 2)
	    // Print out degrees C
	    printf("%2.1f ", (short)(((ReadBuf[1] << 8) | ReadBuf[0]) + 50)/100.0 );
	  else
	    printf("Error: %d\n", rcnt);
	}

      printf("\n");
      fflush(stdout);
      sleep(1);
    }

  bbI2CClose(2);

  gpioTerminate();
}
