#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "unified-controller.h"
#include "i2c-interface.h"

FILE * cpu_temperature_log_file;
pthread_t cpu_temperature_thread;   // the 
bool      termination_signal;       // used to terminate child thread

void * read_cpu_temperature() {

  FILE * input_stream = NULL;
  double temperature  = 0.0;
  
  while (!termination_signal) {
    input_stream = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    fscanf(input_stream, "%lf", &temperature);
    fprintf(cpu_temperature_log_file, "%6.3f\t", temperature / 1000);
    fprintf(cpu_temperature_log_file, "%6.3f\n", (i2c_device -> i2c -> temperature)());
    fflush(stdout);
    fclose(input_stream);
    sleep(1);
  }
  return NULL;
}

bool initialize_temperature_monitoring(char * log_filename) {

  // Attempt to read temperature to make sure sensor works
  FILE * input_stream = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

  if (input_stream == NULL) return false;

  fclose(input_stream);


  // Success reading, create thread to monitor temperature in the background
  termination_signal = false;
  if (pthread_create(&cpu_temperature_thread, NULL, read_cpu_temperature, NULL)) return false;

  
  // Successful initialization, open log file for recording temperature data
  cpu_temperature_log_file = fopen(log_filename, "a");
  fprintf(cpu_temperature_log_file, "\nRecording temperature\nCPU\t%s\n", i2c_device -> identifier);
  
  return true;
}

void terminate_temperature_monitoring() {
  termination_signal = true;
  fclose(cpu_temperature_log_file);
}

/*
int main() {

  initialize_temperature_monitoring("output-temps.txt");
  sleep(60);
  terminate_temperature_monitoring();
  sleep(1);
  return 0;
}
*/

