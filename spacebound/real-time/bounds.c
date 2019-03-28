#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

FILE * file;

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

int batch(long test_duration, long target_ns) {
  
  printf("Starting %ld minute test using %lf ms delays\n", test_duration, 1.0 * target_ns / 1E6);
  
  long start_time;
  long end_time;
  
  //long target_ns = target_ms_halves * 1000000 / 2;
  
  long samples = (test_duration * 60 * 1E9) / target_ns;
  
  for (long i = 0; i < samples; i++) {
    start_time = real_get_time();
    
    real_nano_sleep(target_ns);
    
    end_time = real_get_time();
    
    long duration = end_time - start_time;
    if (duration < 0) duration += 1000000000;
    
    long error = duration - target_ns;
    
    fprintf(file, "%ld\t%ld\t%ld\n", duration, error, target_ns);
  }
}

int main() {
  
  file = fopen("./real-time-bounds.txt", "w");

  for (char i = 1; i <= 9; i++) {
    batch(1, i * 1E5);
  }

  for (char i = 1; i <=15; i++) {
    batch(6, i * 1E6);
  }
  
  /*for (char i = 1; i <= 30; i++) {
    if (i > 10) i++;
    printf("%d\n", i);
    //batch(1, i);
    }*/

  fclose(file);
  
  exit(0);
}
