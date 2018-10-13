
#include <math.h>

#include "quaternion.h"

#define PI 3.14159265


/*This was pulled into the library on 7/31/18 to implement quaternion tracking -> conversion into angles.
  HOPEFULLY this works. - Tyler*/
// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltat, float * q) {
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


void get_DCM_angles(float * angles, float * q) {

  float C[3][3] = {
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f}
  };

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
}

void get_DCM_angles321(float * angles, float * q) {

  float C[3][3] = {
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f}
  };

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
}

