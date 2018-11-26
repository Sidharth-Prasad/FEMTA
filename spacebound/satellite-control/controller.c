
// This file contains most of Tyler's summer work, modified to fit into the restructured project


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "femta.h"
#include "controller.h"
#include "serial.h"
#include "quaternion.h"
#include "graphics.h"
#include "selector.h"
#include "logger.h"
#include "colors.h"

int default_step_size = 0;
int default_time_between = 0;

void configure_ramping_and_pyramid(int * step_size, int * time_between) {

  erase_print_window(1);
  echo();

  // Decide whether to use defaluts for step size and time steps
  
  print(CONTROL_WINDOW, "About to execute. Use defaults below? [y/n]", 5);

  char default_string[256];
  sprintf(default_string, "step size:    %d/255", default_step_size);
  print(CONTROL_WINDOW, default_string, 1);
  sprintf(default_string, "time between: %ds", default_time_between);
  print(CONTROL_WINDOW, default_string, 1);
  print(CONTROL_WINDOW, "", 1);

  char answer[8];
  wgetstr(print_views[CONTROL_WINDOW] -> view -> window, answer);
  stomp_printer(CONTROL_WINDOW, answer, 1);
  
  if (answer[0] == 'n') {
    // User wants to use new default value

    print(CONTROL_WINDOW, "Please enter time between steps:", 5);
    print(CONTROL_WINDOW, "", 1);
    wgetstr(print_views[CONTROL_WINDOW] -> view -> window, default_string);
    stomp_printer(CONTROL_WINDOW, default_string, 1);
    *time_between = atoi(default_string);
    
    print(CONTROL_WINDOW, "Please enter the step size:", 5);
    print(CONTROL_WINDOW, "", 1);
    wgetstr(print_views[CONTROL_WINDOW] -> view -> window, default_string);
    stomp_printer(CONTROL_WINDOW, default_string, 1);
    *step_size = atoi(default_string);
    
    print(CONTROL_WINDOW, "Save these as the new defaults? [y/n]", 5);
    print(CONTROL_WINDOW, "", 1);

    wgetstr(print_views[CONTROL_WINDOW] -> view -> window, answer);
    stomp_printer(CONTROL_WINDOW, answer, 1);

    if (answer[0] == 'y') {
      default_step_size    = *step_size;
      default_time_between = *time_between;

      
      print(GENERAL_WINDOW, "Changed defaults", 5);
      char default_string[256];
      sprintf(default_string, "step size:    %d/255", default_step_size);
      print(GENERAL_WINDOW, default_string, 1);
      sprintf(default_string, "time between: %ds", default_time_between);
      print(GENERAL_WINDOW, default_string, 1);
    }
  }
  noecho();

}

void pause_execution() {
  char pause_buffer[256];
  print(CONTROL_WINDOW, "Press enter to continue", 5);
  print(CONTROL_WINDOW, "", 1);
  wgetstr(print_views[CONTROL_WINDOW] -> view -> window, pause_buffer);
  stomp_printer(CONTROL_WINDOW, pause_buffer, 1);
}

void ramp_up(void * nil) {

  int step_size    = default_step_size;
  int time_between = default_time_between;
  
  configure_ramping_and_pyramid(&step_size, &time_between);
  
  print(CONTROL_WINDOW, "First phase", 5);

  for (int j = 0; j <= 1; j++) {

    for (int i = MIN_SAT; i <= MAX_PWM; i += step_size) {
      set_pwm(QB -> pins + j, i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(time_between);
    }

    set_pwm(QB -> pins + j, 0);
    set_pwm(QB -> pins + j + 2, 0);

    if (!j) {
      pause_execution();
      print(CONTROL_WINDOW, "Second phase", 5);
    }
  }

  present_selector((void *) visible_selector);
}

void pyramid(void * nil) {
  
  int step_size    = default_step_size;
  int time_between = default_time_between;
  
  configure_ramping_and_pyramid(&step_size, &time_between);
  
  print(CONTROL_WINDOW, "First phase", 5);
  
  for (int j = 0; j <= 1; j++) {
    
    for (int i = MIN_SAT; i <= MAX_PWM; i += step_size) {
      set_pwm(QB -> pins + j    , i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(time_between);
    }
    
    for (int i = MAX_PWM; i >= MIN_SAT; i -= step_size) {
      set_pwm(QB -> pins + j    , i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(time_between);
    }
    
    set_pwm(QB -> pins + j, 0);
    set_pwm(QB -> pins + j + 2, 0);
    
    if (!j) {
      pause_execution();
      print(CONTROL_WINDOW, "Second phase", 5);
    }
  }
  
  present_selector((void *) visible_selector);
}

void set_bank_speed(bool Clockwise, int pwm_num) {

  if (Clockwise) {
    set_pwm(QB -> pins + 1, pwm_num);
    set_pwm(QB -> pins + 2, pwm_num);
    return;
  }

  set_pwm(QB -> pins + 0, pwm_num);
  set_pwm(QB -> pins + 3, pwm_num);
}


__attribute__((const)) float rise_time(float phi_des) {
  // estimates the rise time for the system's tracking signal
  return phi_des / 1.0; // linear scale for now, determine actual from testing
}

__attribute__((const)) float tracking_signal_value(int phi_des, float t, float tr) {
  /* Purpose of this is to get a live tracking signal value at any given time since start t
   * phi_des = desired final orientation, degrees
   * t = current time since start in seconds
   * tr = rise time for this system, currently estimated as a function of phi_des.
   */
  float phi_tr;

  if (t <= tr) phi_tr = (phi_des / 2.0) * sin((PI / tr) * t - (PI / 2)) + phi_des / 2.0;
  else         phi_tr = phi_des;

  return phi_tr;
}

/*void PID_controller(bool CW, bool CCW, float init_or, float dor) {
  // init_or will be defined as 0 degrees until data can be read from MPU
  // dor is the change in orientation (delta orientation)
  
  Logger * pid_logger = create_logger("./logs/pid-log.txt");
  pid_logger -> open(pid_logger);
  fprintf(pid_logger -> file,
	  YELLOW "\nRecording PID Data\nDevice\tDevice State\tMPU Measures\tBNO Measures\tSystem Time\n" RESET);
  pid_logger -> close(pid_logger);

  // declare variables
  int mpu_reads = 0;
  int bno_reads = 0;
  struct timeval start_time;
  struct timeval end_time;
  float err1, prev_error, err_sum, new_pwmf;
  int new_pwm;
  float q[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion vector

  // define PID Constants
  float KP = 75;
  float KI = 18.75;
  float KD = 37.5;

  // initialize system
  prev_error = 0.0;
  gettimeofday(&start_time, NULL);
  set_bank_speed(CW, CCW, MAX_PWM); // init in one direction




  float phi_f, phi_mpu, tr, phi_tr, tdiff, t;
  float angles[3] = {0.0f, 0.0f, 0.0f};
  float a1, a2, a3, g1, g2, g3, m1, m2, m3;

  phi_f = init_or + dor;
  tr = rise_time(dor);
  tdiff = 0.0;
  while ((float) tdiff <= tr || err1 > PID_ERR_TOL) {

    gettimeofday(&end_time, NULL);
    tdiff = (float) (timeval_diff(NULL, &end_time, &start_time)) / 1000000.0;
    phi_tr = tracking_signal_value(dor, tdiff, tr);

    if (mpu_logger) mpu_reads = mpu_logger -> values_read;
    // using NED convention (North, East Down)
    a1 = mpu_logger -> ax;
    a2 = mpu_logger -> ay;
    a3 = mpu_logger -> az;
    g1 = mpu_logger -> gx;
    g2 = mpu_logger -> gy;
    g3 = mpu_logger -> gz;
    m1 = mpu_logger -> mx;
    m2 = mpu_logger -> my;
    m3 = mpu_logger -> mz;

    MadgwickQuaternionUpdate(-a3, a2, a1, -g3*PI/180, g2*PI/180, g1*PI/180, m3, m1, m2, tdiff, q); // sensor x n accelerometer = y of the gyro
    get_DCM_angles321(angles, q);

    phi_mpu = angles[1];// yaw_angle(mpu_logger);// angles[1]; // mpu_logger -> gx;// get_mpu_val();
    // printf("%f\n", phi_mpu);
    err1 = phi_mpu - phi_tr;

    if (err1 < 0 && prev_error > 0.0) {
      CW = true;
      CCW = false;
      set_bank_speed(!CW, !CCW, 0); // turn off opposite engines
      prev_error = 0.0;
      err_sum = 0.0;
    }
    else if (err1 > 0 && prev_error > 0.0) {
      CW = false;
      CCW = true;
      set_bank_speed(!CW, !CCW, 0);
      prev_error = 0.0;
      err_sum = 0.0;
    }
    err1 = fabs(err1);
    err_sum = err1 + prev_error;
    new_pwmf = err1 * KP + prev_error * KD + err_sum * KI;
    new_pwm = (int) (new_pwmf);

    if (err1 >= PID_ERR_TOL) { // err1 is already ABS'd, so if less than err tol, then it is close to target
      if      (new_pwm < MIN_SAT) new_pwm = MIN_SAT;
      else if (new_pwm > MAX_PWM) new_pwm = MAX_PWM;
    }
    else new_pwm = 0;
    set_bank_speed(CW, CCW, new_pwm);
    prev_error = err1;

    // log this manual command
    logger -> open(logger);
    if (bno_logger) bno_reads = bno_logger -> values_read;
    gettimeofday(&end_time, NULL);
    fprintf(logger -> file, "QB %d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\n",
	    CCW, new_pwm, mpu_reads, bno_reads,tdiff, angles[0], phi_mpu, angles[2], phi_tr);
    logger -> close(logger);
    }
  
}*/

/*float yaw_angle(struct Logger * mpu_logger) {
  // this is an adaptation of kate's Matlab code.
  float theta, m1, m2, m3;
  // int mpu_reads = 0;
  // if (mpu_logger) mpu_reads = mpu_logger -> values_read;
  m1 = mpu_logger -> mx;
  m2 = mpu_logger -> my;
  m3 = mpu_logger -> mz;

  if (m3 > 0) theta = 90.0  - (atan(m1/m3) * 180.0/PI);
  if (m3 < 0) theta = 270.0 - (atan(m1/m3) * 180.0/PI);
  if (m3 == 0) {
    if (m1 < 0) theta = 180.0;
    else        theta = 0.0;
  }
  return theta;
  }*/

void PID_start(void * target) {
  // Sets up a PID meneuver
  // 
  // Note - this can be called again after PID_stop()

  kp = 0.01;
  ki = 0.00;
  kd = 0.00;

  pid_target = (float) target;
  
  pid_logger = create_logger("./logs/pid-log.txt");
  pid_logger -> open(pid_logger);
  fprintf(pid_logger -> file,
	  
	  YELLOW
	  "\nRecording PID Data"
	  "\nConstants: %f\t%f\t%f\n"
	  "\nTime\tAngle\tVelocity\tQ0\tQ1\tQ2\tQ3\n"
	  RESET,
	  
	  kp, ki, kd);
  
  
  serial_routine = PID_controller;
}

void PID_stop(void * nil) {
  pid_logger -> close(pid_logger);
  serial_routine = NULL;
}

void PID_controller(float angle, float velocity, float time) {
  
  float error = pid_target - angle;

  
  
  
}
