


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>


int main(int argc, char ** args) {
  
  FILE * file = fopen(args[1], "r");

  char buffer[1024];

  long sum = 0;
  long maximum = 0;
  long minimum = LONG_MAX;
  long lines = 0;

  long errors = 0;
  
  while (fgets(buffer, 1024, file)) {

    long value = atol(buffer);
    
    sum += value;
    
    if      (value > maximum) maximum = value;
    else if (value < minimum) minimum = value;
    
    if (value > 1E7 / 2) errors++;
    
    lines++;
  }
  
  fclose(file);

  double avg = (double) sum / lines;
  
  printf("Sum: %ld\n", sum);
  printf("Avg: %lf\n", avg);
  printf("Max: %ld\n", maximum);
  printf("Min: %ld\n", minimum);

  printf("\nErrors: %ld\n", errors);

  FILE * source = fopen(args[1], "r");
  FILE * hourly = fopen("hourly.txt", "w");
  long hourly_max = 0;
  long hourly_count = 0;
  long hours = 0;
  
  while (fgets(buffer, 1024, file)) {
    
    long value = atol(buffer);
    
    if (value > hourly_max) hourly_max = value;
    
    if (++hourly_count == 60 * 100) {
      fprintf(hourly, "%ld, %lf\n", hours++, log(hourly_max));
      
      hourly_count = 0;
      hourly_max = 0;
    }
  }
  
  fclose(source);
  fclose(hourly);
  
  return 0;
}
