
#include <time.h>


#include "time.h"


void real_nano_sleep(long ns) {

  struct timespec time_period;
  time_period.tv_sec  = 0;
  time_period.tv_nsec = ns;

  clock_nanosleep(CLOCK_REALTIME, 0, &time_period, NULL);
}


long real_time_diff(struct timespec * past) {
  
  struct timespec now;

  clock_gettime(CLOCK_REALTIME, &now);

  return (now.tv_sec - past->tv_sec) * 1E9 + (now.tv_nsec - past->tv_nsec);
}
