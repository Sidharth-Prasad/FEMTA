#ifndef COMPILER_GAURD_CONTROLLER
#define COMPILER_GAURD_CONTROLLER

#include <stdbool.h>

#include "logger.h"

// define saturation limits. This is due to the nature of the motors for the quadcopter
#define MIN_SAT 0
#define MAX_PWM 0
#define PID_ERR_TOL 0.05
#define PI 3.14159265

// Functions we want other files to be able to call
void ramp_up(void * nil);
void pyramid(int stepsize, int timebtwn);

void set_bank_speed(bool CW, bool CCW, int pwn_num);

float rise_time(float phi_des);

float tracking_signal_value(int phi_des, float t, float tr);

void PID_controller(bool CW, bool CCW, float init_or, float dor);

Logger * pid_logger;

#endif
