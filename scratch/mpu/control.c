#include <stdio.h>

#include <pigpio.h>

int main(int argc, char *argv[])
{
  double start;

  if (gpioInitialise() < 0)
    {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
    }

  /* Set GPIO modes */
  gpioSetMode(4, PI_OUTPUT);
  gpioSetMode(17, PI_OUTPUT);
  gpioSetMode(18, PI_OUTPUT);
  gpioSetMode(23, PI_INPUT);
  gpioSetMode(24, PI_OUTPUT);

  /* Start 1500 us servo pulses on GPIO4 */
  //gpioServo(4, 1500);

  /* Start 75% dutycycle PWM on GPIO17 */
  //gpioPWM(17, 192); /* 192/255 = 75% */

  start = time_time();

  while ((time_time() - start) < 360.0)
    {
      gpioWrite(4, 1); /* on */
      //gpioWrite(17, 1);
      //gpioWrite(18, 1);
      //gpioWrite(24, 1);

      time_sleep(0.5);

      gpioWrite(4, 0); /* off */
      //gpioWrite(17, 0);
      //gpioWrite(18, 0);
      //gpioWrite(24, 0);

      time_sleep(0.5);

      /* Mirror GPIO24 from GPIO23 */
      //gpioWrite(24, gpioRead(23));
    }

  /* Stop DMA, release resources */
  gpioTerminate();

  return 0;
}
