// System libraries
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
//#include <wiringPi.h>

// Program headers, in compilation order
#include "unified-controller.h"
#include "i2c-interface.h"
#include "serial-interface.h"
#include "temperature-monitoring.h"
#include "graphics.h"
#include "logger.h"
#include "colors.h"

#define NUMBER_OF_MODULES 4

#define I2C_STATE 2
#define UART_STATE 3

//define saturation limits. This is due to the nature of the motors for the quadcopter.
#define MIN_SAT 0 //MIN_SAT and MAX_PWM temporarily 0 until the MPU data can be read accurately
#define MAX_PWM 0
#define PID_ERR_TOL 0.05
#define PI 3.14159265

float yaw_angle(struct Logger * mpu_logger);

void initialize_pin(pin * initialent, char logical, char physical, short state) {
  initialent -> state    = state;
  initialent -> logical  = logical;
  initialent -> physical = physical;

  initialent -> voltage = 0;
  if (state == PI_OUTPUT || state == PI_INPUT) gpioSetMode(logical, state);
  if (state == PI_OUTPUT) gpioWrite(logical, 0);
}

void initialize_satellite() {
  //wiringPiSetupSys(); //needed for micros()
  // Exit if gpio's are unavailable for some reason
  if (gpioInitialise() < 0) {
    printf(RED "a critical error has occured\n" RESET);
    exit(1);
  }
  // Get space for modules
  modules = malloc(NUMBER_OF_MODULES * sizeof(module *));
  char m;
  for (m = 0; m < NUMBER_OF_MODULES; m++) modules[m] = malloc(sizeof(module));
  for (m = 0; m < NUMBER_OF_MODULES; m++) modules[m] -> loaded = false;
  
  // All modules should be grouped together ***FEMTA is now renamed as QB (Quad Bank, 2 banks of 2 engines)
  BNO   = modules[0];
  MPU   = modules[1];
  Valve = modules[2];
  QB    = modules[3];

  // Set module identifiers for printing
  BNO   -> identifier = "BNO 055";
  MPU   -> identifier = "MPU 9250";
  Valve -> identifier = "Valve";
  QB    -> identifier = "Quad Bank";

  // Set each module's number of pins
  BNO   -> n_pins = 3;
  MPU   -> n_pins = 2;
  Valve -> n_pins = 1;
  QB    -> n_pins = 4;

  // Get space for module pin arrays
  for (m = 0; m < NUMBER_OF_MODULES; m++)
    modules[m] -> pins = malloc((modules[m] -> n_pins) * sizeof(module));

  // The BNO has the UART interface ******* Changed 7/12/18 to reflect new board, also not hooked up for the quad sat
  initialize_pin(&(BNO -> pins[0]), 14,  8, UART_STATE);   // UART TXD
  initialize_pin(&(BNO -> pins[1]), 15, 10, UART_STATE);   // UART RXD
  //initialize_pin(&(BNO -> pins[2]), 23, 16, PI_OUTPUT); always on.

  // The MPU has the I2C interface ******** Changed 7/12/18 to reflect new board
  initialize_pin(&(MPU -> pins[0]),  2, 3, I2C_STATE);  // I2C SDA
  initialize_pin(&(MPU -> pins[1]),  3, 5, I2C_STATE);  // I2C SCL

  // The Valve is controlled via digital states
  initialize_pin(&(Valve -> pins[0]), 17, 11, PI_OUTPUT);

  //The quad bank pins are: 0 = CW, 1 = CCW, and 2 is the PWM control. The circuit
  //is set up with the 0 and 1 pins acting as "on" switches; so setting 0 to HIGH
  //will turn on the CW engines, and then 2 will act as the throttle.
  initialize_pin(&(QB -> pins[0]), 23, 16, PI_OUTPUT);
  initialize_pin(&(QB -> pins[1]), 18, 12, PI_OUTPUT);
  initialize_pin(&(QB -> pins[2]), 24, 18, PI_OUTPUT);
  initialize_pin(&(QB -> pins[3]), 25, 22, PI_OUTPUT);

  // Set up the interfaces (TEMPORARY BLOCK OUT TIL PCB WORKS
  bool i2c_success    = initialize_i2c(MPU);
  bool serial_success = initialize_UART(BNO);
  
  // Set each module's initialization state
  BNO   -> initialized = true; //serial_success
  MPU   -> initialized = i2c_success;
  Valve -> initialized = true;
  QB    -> initialized = true;

  bool thermal_success = initialize_temperature_monitoring();
  
  // print information to the user
  printf(GREY "\nInitializing satellite\n\n" RESET);
  if (thermal_success) printf(GREEN "\tCPU\tSUCCESS\tSPAWNED\n" RESET);
  else printf(RED "\tI2C\tFAILURE\t\tUnable to read/log CPU temperature data\n" RESET);

  if (i2c_success) {
    printf(GREEN "\tI2C\tSUCCESS\tSPAWNED\n" RESET);
    printStartupConstants("\t\t");
  }
  else printf(RED "\tI2C\tFAILURE\t\tError: %d\n" RESET, i2cReadByteData(MPU -> i2c -> i2c_address, 0));

  // Serial_success is a highly falible indicator. It's a long story.
  if (BNO -> initialized) printf(GREEN "\tBNO\tSUCCESS\tSPAWNED\n" RESET);
  else                    printf(RED   "\tBNO\tOFFLINE\t" RESET);
  
  printf("\n");
  if (!(i2c_success && thermal_success && serial_success)) {
    printf( RED "\nSatellite failed to initialize" RESET "\n\n");
    return;
  }
  printf(GREEN "\nSatellite initialized successfully!" RESET "\n\n");
  print(0, "Satellite initialized successfully!", 0);
}

void print_configuration() {

  // prints out a table indicating the allocation map
  char m;
  char p;
  printf(GREY "\tlogical   physical   state\n" RESET);
  for (m = 0; m < NUMBER_OF_MODULES; m++) {
    printf(GREY "%s\n" RESET, modules[m] -> identifier);
    for (p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].logical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].logical);
      if (modules[m] -> pins[p].physical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].physical);

      // print out the human-readable state
      printf("         ");
      if (modules[m] -> initialized == false) printf(RED  );
      else                                    printf(GREEN);
      if      (modules[m] -> pins[p].state == PI_INPUT)   printf(RESET "Input" );
      else if (modules[m] -> pins[p].state == PI_OUTPUT)  printf(RESET "Output");
      else if (modules[m] -> pins[p].state == I2C_STATE)  printf(      "I2C"   );
      else if (modules[m] -> pins[p].state == UART_STATE) printf(      "UART"  );
      printf(RESET "\n");
    }
    printf("\n");
  }
}

void terminate_satellite() {

  // Set all output pins to 0 before exit
  char m;
  char p;
  for (m = 0; m < NUMBER_OF_MODULES; m++) {
    for (p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].state == PI_OUTPUT) gpioWrite(modules[m] -> pins[p].logical, 0);
    }
  }
  
  terminate_temperature_monitoring();
  terminate_mpu_logging();
  terminate_bno_logging();
  gpioTerminate();
}

void check_if_writeable(pin * p) {

  // scream bloody hell if this pin is read-only
  if (p -> state == PI_INPUT) {
    printf(RED "read-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

void check_if_readable(pin * p) {

  // scream bloody hell if this pin is write-only
  if (p -> state == PI_OUTPUT) {
    printf(RED "write-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

char read_voltage(pin * p) {

  check_if_readable(p);
  p -> voltage = gpioRead(p -> logical);
  return p -> voltage;
}

void set_voltage(pin * p, char voltage) {
  check_if_writeable(p);
  p -> voltage = voltage;
  gpioWrite(p -> logical, (p -> voltage > 0));
}

void set_pwm(pin * p, unsigned char duty_cycle) {

  check_if_writeable(p);
  p -> duty_cycle = duty_cycle;
  gpioPWM(p -> logical, p -> duty_cycle);
}

void print_main_screen(void){
  erase_print_window(1);
  print(1, "c: cycle graphs"  , 0);
  print(1, "m: manual control", 0);
  print(1, "a: auto control"  , 0);
  print(1, "i: PID control"   , 0);
  print(1, "q: quit"          , 0);

  return;
}

void print_manual_screen(void){
  erase_print_window(1);
  print(1, "b: back"     , 0);
  print(1, "v: valve"    , 0);
  print(1, "w: QB CW"    , 0);
  print(1, "e: QB CCW"   , 0);
  print(1, "r: QB PWM"   , 0);
  print(1, "p: Pump down", 0);

  return;
}   

void print_auto_screen(void){
  erase_print_window(1);
  print(1, "b: back"            , 0);
  print(1, "x: ramp 0-100%"     , 0);
  print(1, "y: pyramid 0-100-0%", 0);
  print(1, "z: configuration"   , 0);

  return;
}

void print_PID_main_screen(void){
  erase_print_window(1);
  print(1, "b: back"                  , 0);
  print(1, "t: test w/ generated data", 0);
  print(1, "n: initialize PID"        , 0);

  return;
}

void ramp_up(int stepsize, int timebtwn){
  int j;
  int i;
  char dummy;
  for(j = 0; j <= 1; j++){
    for(i = MIN_SAT; i <= MAX_PWM; i += stepsize){
      set_pwm(QB -> pins + j, i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(timebtwn);
    }
    set_pwm(QB -> pins + j, 0);
    set_pwm(QB -> pins + j + 2, 0);
    scanf("%s", &dummy);
  }
  return;
}

void pyramid(int stepsize, int timebtwn){
  int j;
  int i;
  char dummy;
  for(j = 0; j <= 1; j++){
    for(i = MIN_SAT; i <= MAX_PWM; i += stepsize){
      set_pwm(QB -> pins + j, i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(timebtwn);
    }
    for(i = MAX_PWM; i >= MIN_SAT; i -= stepsize){
      set_pwm(QB -> pins + j, i);
      set_pwm(QB -> pins + j + 2, i);
      sleep(timebtwn);
    }
    set_pwm(QB -> pins + j, 0);
    set_pwm(QB -> pins + j + 2, 0);
    scanf("%s", &dummy);
  }    
}

void set_bank_speed(bool CW, bool CCW, int pwm_num){

  if(CW){
    set_pwm(QB -> pins + 1, pwm_num);
    set_pwm(QB -> pins + 2, pwm_num);
  }
  else if(CCW){
    set_pwm(QB -> pins + 0, pwm_num);
    set_pwm(QB -> pins + 3, pwm_num);
  }

   return;
}


long long timeval_diff(struct timeval *difference, struct timeval *end_time,
		       struct timeval *start_time){
  struct timeval temp_diff;
  if(difference == NULL) difference = &temp_diff;

  difference -> tv_sec  = end_time -> tv_sec-start_time  -> tv_sec;
  difference -> tv_usec = end_time -> tv_usec-start_time -> tv_usec;

  while(difference->tv_usec < 0){
    difference -> tv_usec += 1000000;
    difference -> tv_sec  -= 1;
  }
  return 1000000LL*difference->tv_sec + difference->tv_usec;
}

float rise_time(float phi_des){
  //estimates the rise time for the system's tracking signal
  return phi_des/1; //linear scale for now, determine actual from testing
}

float tracking_signal_value(int phi_des, float t, float tr){
  /* Purpose of this is to get a live tracking signal value at any given time since start t
   * phi_des = desired final orientation, degrees
   * t = current time since start in seconds
   * tr = rise time for this system, currently estimated as a function of phi_des.
   */
  float phi_tr;

  if(t <= tr) phi_tr = (phi_des/2)*sin((PI/tr)*t - (PI/2)) + phi_des/2;
  else phi_tr = phi_des;

  return phi_tr;
}

float get_mpu_val(){
  FILE* fhand = fopen("./logs/mpu-log.txt", "r");
  char line[1024] = "";
  char c = '1';
  int max_back = -100;
  int i = -2;
  int mpu_read;
  float gx, gy, gz, ax, ay, az, mx, my, mz, quoi; 
  //back up fseek until newline character is read
  while(i >= max_back && c != '\n'){
    fseek(fhand, i, SEEK_END);
    c = fgetc(fhand);
    i--;
  }
  fgets(line, sizeof(line), fhand);
  sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f", &mpu_read, &gx, &gy, &gz, &ax, &ay, &az, &mx, &my, &mz, &quoi);
  fclose(fhand);
  return gx;
}


/*This was pulled into the library on 7/31/18 to implement quaternion tracking -> conversion into angles.
  HOPEFULLY this works. - Tyler*/
// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltat, float *q)
{
  /*
   * IMPORTANT NOTES: Accelerometer and gyroscope have the same axes, but magnetometer does not. the setup looks like this:
   * ax = gx = my; ay = gy = mx; az = gz = -mz
   * gravity is by convention positive-down. This means that for a normal system, gyro/accel North would be +x-axis, East -y-axis, Down -z-axis.
   * BUT, the chip is currently mounted with North being -z, East = +y, Down = +x.
   */
  float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
  float norm;
  float hx, hy, _2bx, _2bz;
  float s1, s2, s3, s4;
  float qDot1, qDot2, qDot3, qDot4;
  
  // Auxiliary variables to avoid repeated arithmetic
  float _2q1mx;
  float _2q1my;
  float _2q1mz;
  float _2q2mx;
  float _4bx;
  float _4bz;
  float _2q1 = 2.0f * q1;
  float _2q2 = 2.0f * q2;
  float _2q3 = 2.0f * q3;
  float _2q4 = 2.0f * q4;
  float _2q1q3 = 2.0f * q1 * q3;
  float _2q3q4 = 2.0f * q3 * q4;
  float q1q1 = q1 * q1;
  float q1q2 = q1 * q2;
  float q1q3 = q1 * q3;
  float q1q4 = q1 * q4;
  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;
  float q2q4 = q2 * q4;
  float q3q3 = q3 * q3;
  float q3q4 = q3 * q4;
  float q4q4 = q4 * q4;
  //next 2 vars are required for Magdwick filtering, stolen from https://github.com/kriswiner/MPU9250/blob/master/MPU9250BasicAHRS.ino
  float GyroMeasError = PI * (40.0f / 180.0f);
  float beta = sqrt(3.0f / 4.0f) * GyroMeasError * 15;

  // Normalise accelerometer measurement
  norm = sqrtf(ax * ax + ay * ay + az * az);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f/norm;
  ax *= norm;
  ay *= norm;
  az *= norm;

  // Normalise magnetometer measurement
  norm = sqrtf(mx * mx + my * my + mz * mz);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f/norm;
  mx *= norm;
  my *= norm;
  mz *= norm;

  // Reference direction of Earth's magnetic field
  _2q1mx = 2.0f * q1 * mx;
  _2q1my = 2.0f * q1 * my;
  _2q1mz = 2.0f * q1 * mz;
  _2q2mx = 2.0f * q2 * mx;
  hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
  hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
  _2bx = sqrtf(hx * hx + hy * hy);
  _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
  _4bx = 2.0f * _2bx;
  _4bz = 2.0f * _2bz;

  // Gradient decent algorithm corrective step
  s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  norm = sqrtf(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
  norm = 1.0f/norm;
  s1 *= norm;
  s2 *= norm;
  s3 *= norm;
  s4 *= norm;

  // Compute rate of change of quaternion
  qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
  qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
  qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
  qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

  // Integrate to yield quaternion
  q1 += qDot1 * deltat;
  q2 += qDot2 * deltat;
  q3 += qDot3 * deltat;
  q4 += qDot4 * deltat;
  norm = sqrtf(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
  norm = 1.0f/norm;
  q[0] = q1 * norm;
  q[1] = q2 * norm;
  q[2] = q3 * norm;
  q[3] = q4 * norm;

}


void get_DCM_angles(float *angles, float *q){
  float C[3][3] = {{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f}};
  //rename quaternions to get appropriate naming convention
  float q1 = q[0];
  float q2 = q[1];
  float q3 = q[2];
  float q4 = q[4];
  float c31, c32, c33, c23, c13;
  float psi, theta, phi, psi11, psi12, psi21, psi22, phi11, phi12, phi21, phi22;
  float angle_tol = 5; //degrees
  
  //RIGHT NOW, we only care about the rotation about the x axis. so we will solve for
  //this with the assumption of a body-thru 3-1-3 rotation matrix.

  //pick C31, C32, C33, C23, C13 coordinates
  C[2][0] = 2 * (q3*q1 - q2*q4);   //C31
  C[2][1] = 2 * (q2*q3 + q1*q4);   //C32
  C[2][2] = 1 - 2*q1*q1 - 2*q2*q2; //C33
  C[1][2] = 2 * (q2*q3 - q1*q4);   //C23
  C[0][2] = 2 * (q3*q1 + q2*q4);   //C13

  //rename for quadrant checks
  c31 = C[2][0];
  c32 = C[2][1];
  c33 = C[2][2];
  c23 = C[1][2];
  c13 = C[0][2];
  
  //quadrant checks
  theta = acos(C[2][2]) * 180/PI;//defaults to the smallest angle - we assume it start within quad I/II

  //c31
  psi11 = asin(c31/sin(theta)) * 180/PI;
  psi12 = 180 - psi11;
  //c32
  psi21 = acos(c32/sin(theta)) * 180/PI;
  psi22 = 360 - psi21;
  //c21
  phi11 = asin(c23/sin(theta)) * 180/PI;
  phi12 = 180 - phi11;
  //c22
  phi21 = acos(-c23/sin(theta)) * 180/PI;
  phi22 = 360 - phi21;

  //get psi
  angles[0] = atan2(c13,c23) * 180/PI;
  //assign theta
  angles[1] = theta;
  //get phi
  angles[2] = -atan2(c31,c32) * 180/PI;
  return;
}

void get_DCM_angles321(float *angles, float *q){
  float C[3][3] = {{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f}};
  //rename quaternions to get appropriate naming convention
  float q1 = q[0];
  float q2 = q[1];
  float q3 = q[2];
  float q4 = q[4];
  float psi, theta, phi;
  float angle_tol = 5; //degrees
  
  //Implement methodology seen in the madgwick_internal_report PDF
  psi = atan2(2*q2*q3 - 2*q1*q4, 2*q1*q1 + 2*q2*q2 - 1);
  theta = -asin(2*q2*q4 + 2*q1*q3);
  phi = atan2(2*q3*q4 - 2*q1*q2, 2*q1*q1 + 2*q4*q4 - 1);
  //get psi
  angles[0] = 2*q2*q4 + 2*q1*q3;//psi * 180/PI;
  //assign theta
  angles[1] = theta * 180/PI;
  //get phi
  angles[2] = phi * 180/PI;
  return;
}


void PID_controller(bool CW, bool CCW, float init_or, float dor, char mode){
  //init_or will be defined as 0 degrees until data can be read from MPU
  //dor is the change in orientation (delta orientation)

  Logger * logger = create_logger("./logs/PID-log.txt");
  logger -> open(logger);
  fprintf(logger -> file,
	  YELLOW "\nRecording PID Data\nDevice\tDevice State\tMPU Measures\tBNO Measures\tSystem Time\n" RESET);
  logger -> close(logger);
  
  //declare variables
  int mpu_reads = 0;
  int bno_reads = 0;
  struct timeval start_time;
  struct timeval end_time;
  float err1, prev_error, err_sum, KP, KI, KD, new_pwmf;
  int new_pwm;
  float q[4] = {1.0f, 0.0f, 0.0f, 0.0f}; //quaternion vector
  
  //define PID Constants
  KP = 75;
  KI = 18.75;
  KD = 37.5;

  //initialize system
  prev_error = 0.0;
  gettimeofday(&start_time, NULL);
  set_bank_speed(CW,CCW,MAX_PWM); //init in one direction

  if(mode == 't'){
    /* static const char filename[] = "error.txt";
    FILE *file = fopen(filename, "r");
    if(file != NULL){
      char line[128], spwm[5];
      char* pch;
      int i;
      
      while(fgets(line, sizeof(line), file) != NULL){ //extracts the data from the txt file
	err1 = atof(line);
	//capture switch over cases
	if(err1 < 0 && prev_error > 0.0){
	  CW = true;
	  CCW = false;
	  set_bank_speed(!CW, !CCW, 0); //turn off opposite engines
	  prev_error = 0.0;
	  err_sum = 0.0;
	}
	else if (err1 > 0 && prev_error > 0.0){
	  CW = false;
	  CCW = true;
	  set_bank_speed(!CW, !CCW, 0);
	  prev_error = 0.0;
	  err_sum = 0.0;
	}
	err1 = fabs(err1);
	err_sum = err1 + prev_error;
        new_pwmf = err1 * KP + prev_error * KD + err_sum * KI;
	new_pwm = (int)(new_pwmf);
	
	if(err1 >= PID_ERR_TOL){ //err1 is already ABS'd, so if less than err tol, then it is close to target
	  if(new_pwm < MIN_SAT) new_pwm = MIN_SAT;
	  else if(new_pwm > MAX_PWM) new_pwm = MAX_PWM;
	}
	else new_pwm = 0;
	set_bank_speed(CW, CCW, new_pwm);
	prev_error = err1;
	
	//log  this manual command
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	if (bno_logger) bno_reads = bno_logger -> values_read;
        gettimeofday(&end_time, NULL);
	fprintf(logger -> file, "QB %d\t%d\t%d\t%d\t%lf\n",
		CCW, new_pwm, mpu_reads, bno_reads,((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
	logger -> close(logger);
      
	nano_sleep(1000000); //ms resolution
      }
    }
    fclose(file);
    */
  }//end mode if
    
  
  else{
    float phi_f, phi_mpu, tr, phi_tr, tdiff, t;
    float angles[3] ={0.0f, 0.0f, 0.0f};
    float a1, a2, a3, g1, g2, g3, m1, m2, m3;
    
    phi_f = init_or + dor;
    tr = rise_time(dor);
    tdiff = 0.0;
    while((float)tdiff <= tr || err1 > PID_ERR_TOL){

      gettimeofday(&end_time, NULL);
      tdiff = (float)(timeval_diff(NULL, &end_time, &start_time))/1000000;
      phi_tr = tracking_signal_value(dor, tdiff, tr);

      if (mpu_logger) mpu_reads = mpu_logger -> values_read;
      //using NED convention (North, East Down)
      a1 = mpu_logger -> ax;
      a2 = mpu_logger -> ay;
      a3 = mpu_logger -> az;
      g1 = mpu_logger -> gx;
      g2 = mpu_logger -> gy;
      g3 = mpu_logger -> gz;
      m1 = mpu_logger -> mx;
      m2 = mpu_logger -> my;
      m3 = mpu_logger -> mz;

      MadgwickQuaternionUpdate(-a3, a2, a1, -g3*PI/180, g2*PI/180, g1*PI/180, m3, m1, m2, tdiff, q); //sensor x n accelerometer = y of the gyro
      get_DCM_angles321(angles, q);

      phi_mpu = angles[1];//yaw_angle(mpu_logger);//angles[1]; //mpu_logger -> gx;//get_mpu_val();
      //printf("%f\n", phi_mpu);
      err1 = phi_mpu - phi_tr;
      
      if(err1 < 0 && prev_error > 0.0){
	  CW = true;
	  CCW = false;
	  set_bank_speed(!CW, !CCW, 0); //turn off opposite engines
	  prev_error = 0.0;
	  err_sum = 0.0;
	}
      else if (err1 > 0 && prev_error > 0.0){
	CW = false;
	CCW = true;
	set_bank_speed(!CW, !CCW, 0);
	prev_error = 0.0;
	err_sum = 0.0;
      }
      err1 = fabs(err1);
      err_sum = err1 + prev_error;
      new_pwmf = err1 * KP + prev_error * KD + err_sum * KI;
      new_pwm = (int)(new_pwmf);
      
      if(err1 >= PID_ERR_TOL){ //err1 is already ABS'd, so if less than err tol, then it is close to target
	if(new_pwm < MIN_SAT) new_pwm = MIN_SAT;
	else if(new_pwm > MAX_PWM) new_pwm = MAX_PWM;
      }
      else new_pwm = 0;
      set_bank_speed(CW, CCW, new_pwm);
      prev_error = err1;
      
      //log  this manual command
      logger -> open(logger);
      if (bno_logger) bno_reads = bno_logger -> values_read;
      gettimeofday(&end_time, NULL);
      fprintf(logger -> file, "QB %d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\n",
	      CCW, new_pwm, mpu_reads, bno_reads,tdiff, angles[0], phi_mpu, angles[2], phi_tr);
      logger -> close(logger);
    }
  }   
  return;
}

float yaw_angle(struct Logger * mpu_logger){
  //this is an adaptation of kate's Matlab code.
  float theta, m1, m2, m3;
  //int mpu_reads = 0;
  //if (mpu_logger) mpu_reads = mpu_logger -> values_read;
  m1 = mpu_logger -> mx;
  m2 = mpu_logger -> my;
  m3 = mpu_logger -> mz;
  
  if(m3 > 0) theta = 90  - (atan(m1/m3) * 180/PI);
  if(m3 < 0) theta = 270 - (atan(m1/m3) * 180/PI);
  if(m3 == 0){
    if(m1 < 0) theta = 180;
    else theta = 0;
  }
  return theta;
}


int main() {
  struct timeval start_time;
  struct timeval end_time;
  // time_t start_time = time(NULL);
  gettimeofday(&start_time, NULL);
  
  // Create the control logger
  Logger * logger = create_logger("./logs/control-log.txt");
  logger -> open(logger);
  fprintf(logger -> file,
	  YELLOW "\nRecording Control Data\nDevice\tDevice State\tMPU Measures\tBNO Measures\tSystem Time\n" RESET);
  logger -> close(logger);

  initialize_satellite();
  print_configuration();
  initialize_graphics();
  
  Plot * all_possible_owners[8] = {
    temperature_plot,
    mpu_gyro_plot,
    mpu_acel_plot,
    mpu_magn_plot,
    bno_gyro_plot,
    bno_acel_plot,
    bno_lina_plot,
    bno_magn_plot,
  };

  List * owner_index_list = create_list(8);

  // Temperature plot no matter what
  list_insert(owner_index_list, create_inode(0));
  
  Node * graph_owner_index_node = owner_index_list -> head;
  graph_owner = all_possible_owners[graph_owner_index_node -> ivalue];

  char input;
  bool user_input = true;
  bool manual_mode = false;
  bool man_quad = false;
  bool auto_mode = false;
  bool bconfig = false;
  bool PID_main = false;
  bool PID_sub = false;
  bool val = false;
  bool CW = false;
  bool CCW = false;
  char input2;
  while (user_input) {
    
    input = getc(stdin);

    int mpu_reads = 0;
    int bno_reads = 0;	
    char number;
    char number3;
    if(!manual_mode) set_pwm(QB -> pins + 0, 0); //trying to make this start up at zero, currently on
    if (manual_mode) {
      switch (input) {
      case 'w':
      case 'e':
      case 'r':
    
	; // Epsilon
	if      (input == 'w'){
	  input2 = '0';
	  if(!CW) CW = true;
	  else CW = false; 
	}
	else if (input == 'e'){
	  input2 = '1';
	  if(!CCW) CCW = true;
	  else CCW = false;
	}
	else if (input == 'r') input2 = '2';

	number = input2 - '0';    // The actual number pressed 
   
	// Setup functions
	//if (input == 'w')  set_pwm(QB -> pins + number, 100);
	//else set_voltage(QB -> pins + number, 1);

	//if((QB -> pins + number) -> duty_cycle) set_pwm(QB -> pins + number, 0);
	//else                                    set_pwm(QB -> pins + number,MAX_PWM);
	
	// Show state change to user
        update_state_graphic(18 + 0, CW);
	update_state_graphic(18 + 1, CCW);
	if(input == 'r') update_state_graphic(18 + 2, 1);
        //log  this manual command
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	if (bno_logger) bno_reads = bno_logger -> values_read;
	//fprintf(logger -> file, "QB %d\t%d\t%d\t%d\t%d\n",
	//	number, (QB -> pins + number) -> duty_cycle, mpu_reads, bno_reads, time(NULL) - start_time);
	gettimeofday(&end_time, NULL);
	fprintf(logger -> file, "QB %d\t%d\t%d\t%d\t%lf\n",
		number, (QB -> pins + number) -> duty_cycle, mpu_reads, bno_reads,((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
	logger -> close(logger);
      
	break;
	
      case 'v':
	
	// Flip valve voltage
	Valve -> pins -> voltage = !Valve -> pins -> voltage;
	set_voltage(Valve -> pins, Valve -> pins -> voltage);
	update_state_graphic(15, Valve -> pins -> voltage);

	// Log this manual command
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	if (bno_logger) bno_reads = bno_logger -> values_read;
	//fprintf(logger -> file, "Valve\t%d\t%d\t%d\t%d\n",
	//	Valve -> pins -> voltage, mpu_reads, bno_reads, time(NULL) - start_time);
	gettimeofday(&end_time, NULL);
	fprintf(logger -> file, "Valve\t%d\t%d\t%d\t%d\t%lf\n",
		number, (QB -> pins + number) -> duty_cycle, mpu_reads, bno_reads,((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
	logger -> close(logger);
	
	break;
      case 'p':
	
	// Log the pump down message
	logger -> open(logger);
	if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	if (bno_logger) bno_reads = bno_logger -> values_read;
	//fprintf(logger -> file, "Pump\t%d\t%d\t%d\t%d\n", 1, mpu_reads, bno_reads, time(NULL) - start_time);
	gettimeofday(&end_time, NULL);
	fprintf(logger -> file, "Pump\t%d\t%d\t%d\t%d\t%lf\n",
		number, (QB -> pins + number) -> duty_cycle, mpu_reads, bno_reads,((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
	logger -> close(logger);
      }
    }
   
    switch (input) {
      char p;
      char pwm_val[4] = "0   ";
      int j = 0;
      int pwm_num;
      int i;
      float dor;
      char dummy;
      char timestep[3] = "1  ";
      char stepsize[3] = "5  ";
      char dorient[4];
      char shortest[2];
      
    case 'c':
      // Add BNO plots
      if (BNO -> initialized && !BNO -> loaded) {
	for (p = 4; p <= 7; p++) list_insert(owner_index_list, create_inode(p));
	BNO -> loaded = true;
      }

      // Add MPU plots
      if (MPU -> initialized && !MPU -> loaded) {
	for (p = 1; p <= 3; p++) list_insert(owner_index_list, create_inode(p));
	MPU -> loaded = true;
      }
      
      graph_owner_index_node = graph_owner_index_node -> next;
      graph_owner = all_possible_owners[graph_owner_index_node -> ivalue];
      break;

    case 'm':
      print_manual_screen();
      manual_mode = true;
      
      break;

    case 'r':
      //print(0, "r section", 1); //diagnostic
      man_quad = true;
      while(atoi(pwm_val) != -1){
	erase_print_window(1);
	print(1, "b: back", 0);
 	print(1, "enter PWM value (0-255): ", 0);
	//print out the previously entered value
	if (j != 0) print(1, pwm_val, 0);
	j += 1;
	fgets(pwm_val, sizeof(pwm_val), stdin) ;
	pwm_num = atoi(pwm_val);
	if(pwm_num < -1 || pwm_num > MAX_PWM){
	  print(1, "Value exceeds bounds.", 0);
	  sleep(2);
	}
	else if(pwm_num == -1){
	  print(1, "Exiting entry system. press 'b' to go back.", 0);
	  j = 0;
	  set_pwm(QB -> pins + number, 0);
	  set_bank_speed(CW, CCW, 0);
	  // Show state change to user
	  update_state_graphic(18 + number    , 0);
	  if(CW){
	    CW = false;
	    update_state_graphic(18, 0);
	  }
	  else{
	    CCW = false;
	    update_state_graphic(18 + '1' - '0', 0);
	  }
	}
	else{
	  set_bank_speed(CW, CCW, pwm_num);
	  logger -> open(logger);
	  if (mpu_logger) mpu_reads = mpu_logger -> values_read;
	  if (bno_logger) bno_reads = bno_logger -> values_read;
	  // fprintf(logger -> file, "QB\t%d\t%d\t%d\t%d\t%d\n",
	  // number, pwm_num, mpu_reads, bno_reads, time(NULL) - start_time);
	  gettimeofday(&end_time, NULL);
	fprintf(logger -> file, "QB\t%d\t%d\t%d\t%d\t%lf\n",
		!CW, (QB -> pins + number3) -> duty_cycle, mpu_reads, bno_reads,((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
	logger -> close(logger);
	}
      }
      break;
      
    case 'a':
      auto_mode = true;
      print_auto_screen();
      break;
      
    case 'x':
      print(1, "Ramping sequence started. press 'y' when s/c stationary.", 0);
      ramp_up(atoi(stepsize), atoi(timestep)); //runs CW, CCW ramp sequence with current config values
      print_auto_screen(); //reset window
      break;

    case 'y':
      print(1, "Pyramid sequence started. press 'y' when s/c stationary.", 0);
      pyramid(atoi(stepsize), atoi(timestep));
      print_auto_screen(); //reset window
      break;

    case 'z':
      bconfig = true;
      erase_print_window(1);
      print(1, "Time between steps: ", 0);
      fgets(timestep, sizeof(timestep), stdin) ;
      print(1, timestep, 0);
      print(1, "Step size: ", 0);
      fgets(stepsize, sizeof(stepsize), stdin);
      print(1, stepsize, 0);
      print(1, "b: back", 0);
      break;


    case 'i':
      PID_main = true;
      print_PID_main_screen();
      break;

    case 't':
      PID_controller(CW, CCW, 0.0, 135.0, 't');
      break;
      
    case 'n':
      //eventually, I want to be able to run the PID constantly while the rest of the processes can be done;
      //For example, enter 90 degrees as a desired orientation, and then have the PID continue to help hold 90 degrees
      //until the user puts in another desired orientation. Useful for longterm holding of orientations.
      //also, a live view of the current orientation would be useful.
      PID_sub = true;
      val = false;
      while(!val){
	erase_print_window(1);
	print(1,"Enter a desired change in orientation [-360,360]: ", 0);
	fgets(dorient, sizeof(dorient), stdin);
	dor = atof(dorient);
	if(fabs(dor) <= 360) val = true;
	else{
	  print(1, "Invalid input.", 0);
	  sleep(2);
	}
      }
      print(1, dorient, 0);
      print(1, "Shortest route (y/n)? ", 0);
      fgets(shortest, sizeof(shortest), stdin);
      if(shortest[0] != 'y' && shortest[0] != 'n') shortest[0] = 'n'; //people dont follow instructions, default to longest route.
      print(1, shortest, 0);
      nano_sleep(500000000); //sleep for half a second
      PID_controller(CW, CCW, 0.0, dor, 'n'); 
      print(1, "b: back", 0);
      
      break;
      
    case 'b':
      if (manual_mode && !man_quad){
	manual_mode = false;
	print_main_screen();
      }
      else if (manual_mode && man_quad){
	man_quad = false;
	strcpy(pwm_val, "0");
        print_manual_screen();
      }
      else if (auto_mode && !bconfig){
	auto_mode = false;
	print_main_screen();
      }
      else if(bconfig){
	bconfig = false;
	print_auto_screen();
      }
      else if(PID_main && !PID_sub){
	PID_main = false;
	print_main_screen();
      }
      else if(PID_sub){
	PID_sub = false;
	print_PID_main_screen();
      }
      break;
      
    case 'q':
      user_input = false;
      break;
    }
  }

  printf("\n");

  terminate_satellite();
  terminate_graphics();
  
  logger -> open(logger);
  gettimeofday(&end_time, NULL);
  fprintf(logger -> file, YELLOW "\nTerminated gracefully at time %lf seconds" RESET "\n\n", ((double)timeval_diff(NULL, &end_time, &start_time ))/1000000);
  logger -> close(logger);
  logger -> destroy(logger);
  
  return 0;
}




