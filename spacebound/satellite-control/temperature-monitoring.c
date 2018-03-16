#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "unified-controller.h"
#include "i2c-interface.h"
#include "colors.h"

FILE * cpu_temperature_log_file;
char * temperature_log_filename = "./logs/cpu-temperature-log.txt";
pthread_t cpu_temperature_thread;
bool      termination_signal;       // used to terminate child thread
int values_read = 0;

void * read_cpu_temperature() {

  FILE * input_stream = NULL;
  double temperature  = 0.0;
  
  while (!termination_signal) {
    input_stream = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    cpu_temperature_log_file = fopen(temperature_log_filename, "a");
    fscanf(input_stream, "%lf", &temperature);
    fprintf(cpu_temperature_log_file, "%d\t", values_read++);
    fprintf(cpu_temperature_log_file, "%6.3f\t", temperature / 1000);
    if (i2c_device    -> initialized) fprintf(cpu_temperature_log_file, "%6.3f\t", (i2c_device    -> i2c  -> temperature)());
    if (serial_device -> initialized) fprintf(cpu_temperature_log_file, "%6.3f\t", (serial_device -> uart -> temperature)());
    fprintf(cpu_temperature_log_file, "\n");
    fflush(stdout);
    fclose(cpu_temperature_log_file);
    fclose(input_stream);
    sleep(1);
  }
  return NULL;
}

bool initialize_temperature_monitoring() {

  // Attempt to read temperature to make sure sensor works
  FILE * input_stream = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

  if (input_stream == NULL) return false;

  fclose(input_stream);


  // Success reading, create thread to monitor temperature in the background
  termination_signal = false;
  if (pthread_create(&cpu_temperature_thread, NULL, read_cpu_temperature, NULL)) return false;
  
  // Successful initialization, open log file for recording temperature data
  cpu_temperature_log_file = fopen(temperature_log_filename, "a");
  fprintf(cpu_temperature_log_file, RED "\nRecording temperature\nTIME\tCPU\tMPU\tBNO\n" RESET);
  fclose(cpu_temperature_log_file);
  
  return true;
}

void terminate_temperature_monitoring() {
  termination_signal = true;
  //fclose(cpu_temperature_log_file);
}
