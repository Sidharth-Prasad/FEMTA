#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "femta.h"
#include "i2c-interface.h"
#include "temperature.h"
#include "linked-list.h"
#include "graphics.h"
#include "logger.h"
#include "colors.h"

pthread_t cpu_temperature_thread;
bool termination_signal;       // used to terminate child thread

void * read_cpu_temperature() {
  
  FILE * input_stream = NULL;
  double temperature  = 0.0;

  Logger * temperature_logger = create_logger("./logs/cpu-log.txt");
  temperature_logger -> open(temperature_logger);
  fprintf(temperature_logger -> file, RED "\nRecording temperature\nTIME\tCPU\tMPU\n" RESET);  

  /*if (temperature_plot == NULL) {
    temperature_plot = malloc(sizeof(Plot));
    temperature_plot -> name = "     Temperatures v.s. Time     ";
    temperature_plot -> number_of_lists = 1;// + (i2c_device -> initialized) + (serial_device -> initialized);
    temperature_plot -> lists = malloc(temperature_plot -> number_of_lists * sizeof(List *));
    temperature_plot -> has_data = false;
    for (int l = 0; l < temperature_plot -> number_of_lists; l++) {
      temperature_plot -> lists[l] = create_list(number_of_data_points_plottable, true, false);   // Might not have been set
    }
    }*/

  Plot * temperature_plot = (Plot *) CPU -> plots -> head -> value;
  
  while (!termination_signal) {
    input_stream = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    
    fscanf(input_stream, "%lf", &temperature);
    fprintf(temperature_logger -> file, "%d\t", temperature_logger -> values_read++);

    // CPU temperature record

    float reported = temperature / 1000.0;
    
    fprintf(temperature_logger -> file, "%6.3f\t", reported);
    plot_add_value(temperature_plot, temperature_plot -> lists[0], create_node((void *)(*((int *) &reported))));
    
    /*
    // I2C temperature record
    if (i2c_device    -> initialized) {
      float value = (i2c_device    -> i2c  -> temperature)();
      fprintf(cpu_temperature_log_file, "%6.3f\t", value);
      plot_add_value(temperature_plot, temperature_plot -> lists[1], create_fnode(value));
      }*/

    fprintf(temperature_logger -> file, "\n");
    if (!(temperature_logger -> values_read % 16)) fflush(temperature_logger -> file);

    graph_plot(temperature_plot);
    
    fclose(input_stream);
    sleep(1);
  }
  temperature_logger -> close(temperature_logger);
  
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
  
  /*// Successful initialization, open log file for recording temperature data
  cpu_temperature_log_file = fopen(temperature_log_filename, "a");
  fprintf(cpu_temperature_log_file, RED "\nRecording temperature\nTIME\tCPU\tMPU\n" RESET);
  fclose(cpu_temperature_log_file);*/
  
  return true;
}

void terminate_temperature_monitoring() {
  termination_signal = true;
  //fclose(cpu_temperature_log_file);
}
