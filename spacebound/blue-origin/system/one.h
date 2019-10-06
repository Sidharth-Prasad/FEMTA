#ifndef HEADER_GAURD_ONE
#define HEADER_GAURD_ONE

#include <pthread.h>
#include <stdbool.h>

#include "../sensors/sensor.h"
#include "../structures/list.h"
#include "../types/types.h"
#include "../types/thread-types.h"

typedef struct Sensor Sensor;
typedef struct ProtoSensor ProtoSensor;
typedef struct one_device one_device;

typedef bool (*one_reader)(one_device * one);

typedef struct one_device {
  
  Sensor * sensor;
  
  char * path;             // where to get data
  FILE * log;              // log file
  
  int interval;            // time span between schedules in ms
  int hertz;               // reads per second
  int hertz_denominator;   // engenders fractional frequency through deferrals
  int bus_deferrals;       // how many bus deferrals since last read
  int count;               // counts since last read
  
  int total_reads;         // total times this sensor has been read
  
  one_reader read;
  
} one_device;

void init_one();
void start_one();
void terminate_one();

one_device * create_one_device(Sensor * sensor, ProtoSensor * proto,
			       char * path, char * log_path, one_reader read);

void one_close(one_device * one);

#endif
