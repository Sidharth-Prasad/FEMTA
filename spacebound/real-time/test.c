#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

/*void set_realtime_priority() {
  

  pthread_t this_thread = pthread_self();

  struct sched_param params;

  params.sched_priority = 32;

  if (pthread_setschedparam(this_thread, SCHED_FIFO, &params)) {
    printf("Unable to change priority\n");
    exit(2);
  }
  int policy;
  
  if (pthread_getschedparam(this_thread, &policy, &params)) {
    printf("Unable to read priority\n");
    exit(2);
  }

  if (policy == SCHED_FIFO) printf("Scheduling is SCHED_FIFO\n");
  else                      printf("Scheduling is not SCHED_FIFO\n");

  printf("New priority is %d\n", params.sched_priority);
  }*/

void real_nano_sleep(long ns) {
  
  struct timespec time_period;
  time_period.tv_sec  = 0;
  time_period.tv_nsec = ns;

  clock_nanosleep(CLOCK_REALTIME, 0, &time_period, NULL);
}

long real_get_time() {
  struct timespec time_period;
  
  clock_gettime(CLOCK_REALTIME, &time_period);
  
  return time_period.tv_nsec;
}

int main() {
  
  long test_duration = 1;    // length of test in minutes
  
  printf("Starting %ld minute test\n", test_duration);
  
  long start_time;
  long end_time;

  long target_ms = 5;
  long target_ns = target_ms * 1000000;
  
  long samples = (test_duration * 60 * 1000) / target_ms;

  
  long sum_error = 0;
  
  for (long i = 0; i < samples; i++) {
    start_time = real_get_time();
    
    real_nano_sleep(target_ns);
    
    end_time = real_get_time();

    long duration = end_time - start_time;
    if (duration < 0) duration += 1000000000;

    long error = duration - target_ns;

    sum_error += error;
    
    printf("%ld\t%ld\n", duration, error);
  }
  printf("\n");

  printf("Average: %lf\n", (double) sum_error / samples);
  
  printf("Test concluded\n");
  
  exit(0);
}
