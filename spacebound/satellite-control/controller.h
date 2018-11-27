#ifndef COMPILER_GAURD_CONTROLLER
#define COMPILER_GAURD_CONTROLLER

#include <stdbool.h>

#include "logger.h"

// Define saturation limits. This is due to the nature of the motors for the quadcopter
#define MIN_SAT 75
#define MAX_PWM 255
#define PID_ERR_TOL 0.05
#define PI 3.1415926535897932

// Functions we want other files to be able to call
void ramp_up(void * nil);
void pyramid(void * nil);

void set_bank_speed(bool Clockwise, int pwn_num);

float rise_time(float phi_des)                              __attribute__((const));
float tracking_signal_value(int phi_des, float t, float tr) __attribute__((const));

void PID_start(void * target);                                   // Target angle
void PID_stop(void * nil);
void PID_controller(float angle, float velocity, float time);    // Asynchronous loop

bool initialize_PID();    // Sets constants 

// PID constants
float kp;            // Proportional gain
float ki;            // Integral gain
float kd;            // Derivative gain

float pid_target;    // Angle control loop seeks

bool pid_active;     // PID is under way

Logger * pid_logger;

#endif
