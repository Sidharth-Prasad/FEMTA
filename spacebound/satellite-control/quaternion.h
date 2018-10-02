#ifndef COMPILER_GAURD_QUATERNION
#define COMPILER_GAURD_QUATERNION




void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltat, float * q);

void get_DCM_angles   (float * angles, float * q);
void get_DCM_angles321(float * angles, float * q);



#endif
