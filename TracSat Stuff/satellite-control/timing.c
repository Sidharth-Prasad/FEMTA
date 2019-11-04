
#include <time.h>

#include "timing.h"

void nano_sleep(long duration) {
  struct timespec delay, result;
  delay.tv_sec = 0;
  delay.tv_nsec = duration;
  nanosleep(&delay, &result);
}
