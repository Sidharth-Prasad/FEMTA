
/**
 * The following program is a C port of the code located at
 * https://github.com/kriswiner/MPU9250/blob/master/MPU9250_MS5637_AHRS_t3.ino.
 *
 * Alterations have been made by Noah Franks to integrate the file into the FEMTA Cubesat
 * program. Additional code exists for specific use within FEMTA's project requirments,
 * but many of the functions can be copied as they are over to future projects involving 
 * communication with the MPU 9250 over I2C.
 */

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pigpio.h>
#include <pthread.h>
#include <stdbool.h>

#include "unified-controller.h"
#include "i2c-interface.h"
#include "linked-list.h"
#include "graphics.h"
#include "colors.h"

#define AK8963_ST1       0x02
#define AK8963_XOUT_L    0x03
#define AK8963_CNTL      0x0A
#define AK8963_ADDRESS   0x0C
#define AK8963_ASAX      0x10
#define SMPLRT_DIV       0x19
#define CONFIG           0x1A
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define ACCEL_XOUT_H     0x3B
#define TEMP_OUT_H       0x41
#define TEMP_OUT_L       0x42
#define GYRO_XOUT_H      0x43
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B  // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define MPU9250_ADDRESS  0x68
#define FIFO_COUNTH      0x72
#define FIFO_R_W         0x74
#define XA_OFFSET_H      0x77
#define YA_OFFSET_H      0x7A
#define ZA_OFFSET_H      0x7D

FILE * mpu_log_file;
char * mpu_log_file_name = "./logs/mpu-log.txt";
pthread_t mpu_thread;
bool mpu_termination_signal;       // used to terminate child thread
int mpu_values_read = 0;


float gyroBias[3]  = {0, 0, 0};   // Gyro bias calculated at startup
float accelBias[3] = {0, 0, 0};   // Accel bias calculated at startup
float magBias[3] = {0, 0, 0};     // Magn bias calculated at startup

float magScale[3] = {1, 1, 1};        // Needs calibration to make sense
float magCalibration[3] = {0, 0, 0};  // Factory mag calibration and mag bias

enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};

enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

enum Mscale {
  MFS_14BITS = 0, // 0.6 mG per LSB
  MFS_16BITS      // 0.15 mG per LSB
};

uint8_t Ascale = AFS_2G;
uint8_t Gscale = GFS_250DPS;
uint8_t Mscale = MFS_16BITS;   // Choose either 14 or 16-bit magnetometer mode

uint8_t Mmode = 0x02; // 0x02 for 8 Hz, 0x06 for 100 Hz continuous magnetometer data read

float aRes = 2.0   / 32768.0;
float gRes = 250.0 / 32768.0;
float mRes = 10. * 4912. / 32760.0; // higher resolution milliGauss scale

bool newMagData = false;

void printBias(char * offset, char axis, float value) {
  printf(DIM "%s\t%c: " UNDIM BLUE, offset, axis);
  if (value >= 0.0) printf(" ");
  printf("%f\n" RESET, value);
}

void printStartupConstants(char * offset) {
  printf(DIM "%sGyro Biases\n", offset);
  printBias(offset, 'X', gyroBias[0]);
  printBias(offset, 'Y', gyroBias[1]);
  printBias(offset, 'Z', gyroBias[2]);
  
  printf(DIM "%sAccel Biases\n", offset);
  printBias(offset, 'X', accelBias[0]);
  printBias(offset, 'Y', accelBias[1]);
  printBias(offset, 'Z', accelBias[2]);
}

void readBytes(uint8_t address, uint8_t location, uint8_t number, uint8_t * data) {

  // Read the bytes sequentially, writing them to the data array
  for (uint8_t offset = 0; offset < number; offset++) {
    data[offset] = i2cReadByteData(address, location + offset);
  }
}


float readTempData() {
  uint8_t low_bits  = i2cReadByteData(i2c_device -> i2c -> i2c_address, TEMP_OUT_L);
  uint8_t high_bits = i2cReadByteData(i2c_device -> i2c -> i2c_address, TEMP_OUT_H);
  int16_t bytes = (int16_t) (((int16_t) high_bits) << 8 | low_bits);  // Create 2-byte value
  return ((float) bytes) / 333.87 + 21.0; //  Convert to Centigrade
}

void readGyroData(float * axes) {
  uint8_t rawData[6];  // x/y/z gyro register data stored here
  int16_t gyroCount[3];
  readBytes(i2c_device -> i2c -> i2c_address, GYRO_XOUT_H, 6, &rawData[0]);  // Read the 6 data registers into data array
  gyroCount[0] = ((int16_t) rawData[0] << 8) | rawData[1];  // Turn the MSB and LSB into a signed 16-bit value
  gyroCount[1] = ((int16_t) rawData[2] << 8) | rawData[3];
  gyroCount[2] = ((int16_t) rawData[4] << 8) | rawData[5];

  for (int8_t i = 0; i < 3; i++) axes[i] = (float) gyroCount[i] * gRes;
  //for (int8_t i = 0; i < 3; i++) axes[i] = (float) gyroCount[i] * gRes - gyroBias[i];
  //for (int8_t i = 0; i < 3; i++) axes[i] = ((float) gyroCount[i] - gyroBias[i]) * gRes;
}

void readAccelData(float * axes) {
  uint8_t rawData[6];  // x/y/z accel register data stored here
  int16_t accelCount[3];
  readBytes(i2c_device -> i2c -> i2c_address, ACCEL_XOUT_H, 6, &rawData[0]);  // Read the 6 data registers into data array
  accelCount[0] = ((int16_t) rawData[0] << 8) | rawData[1];  // Turn the MSB and LSB into a signed 16-bit value
  accelCount[1] = ((int16_t) rawData[2] << 8) | rawData[3];
  accelCount[2] = ((int16_t) rawData[4] << 8) | rawData[5];

  //for (int8_t i = 0; i < 3; i++) axes[i] = (float) (accelCount[i]-accelBias[i]) * aRes;
  for (int8_t i = 0; i < 3; i++) axes[i] = (float) accelCount[i] * aRes - accelBias[i];
}

void readMagData(float * axes) {
  
  uint8_t rawData[7];  // x y z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
  int16_t magCount[3];
  newMagData = (i2cReadByteData(i2c_device -> i2c -> i2c_slave_address, AK8963_ST1) & 0x01);
  if (newMagData == true) { // wait for magnetometer data ready bit to be set
    // Read the 6 raw data and ST2 registers sequentially into the data array
    readBytes(i2c_device -> i2c -> i2c_slave_address, AK8963_XOUT_L, 7, &rawData[0]);
    uint8_t c = rawData[6]; // End data read by reading ST2 register
    if(!(c & 0x08)) { // Check if magnetic sensor overflow set, if not then report data
      magCount[0] = ((int16_t) rawData[1] << 8) | rawData[0] ;  // Turn the MSB and LSB into a signed 16-bit value
      magCount[1] = ((int16_t) rawData[3] << 8) | rawData[2] ;  // Data stored as little Endian
      magCount[2] = ((int16_t) rawData[5] << 8) | rawData[4] ;
    }
  }

  // Without calibration, this could be problematic
  for (int8_t i = 0; i < 3; i++) axes[i] = ((float) magCount[i] * mRes * magCalibration[i] - magBias[i]) * magScale[i];
}

void nano_sleep(long duration) {
  struct timespec delay, result;
  delay.tv_sec = 0;
  delay.tv_nsec = duration;
  nanosleep(&delay, &result);
}

void * log_mpu_data() {

  while (!mpu_termination_signal) {

    mpu_log_file = fopen(mpu_log_file_name, "a");

    float log_data[50][9];
    
    for (unsigned char i = 0; i < 50; i++) {
      
      readGyroData(log_data[i]);
      readAccelData(log_data[i] + 3);
      readMagData(log_data[i] + 6);
      
      fprintf(mpu_log_file, "%d\t", mpu_values_read++);
      for (unsigned char f = 0; f < 6; f++) {
	fprintf(mpu_log_file, "%.3f\t", log_data[i][f]);
      }
      for (unsigned char f = 0; f < 3; f++) {
	plot_add_value(mpu_gyro_plot, mpu_gyro_plot -> lists[f], create_fnode(log_data[i][f]));
	plot_add_value(mpu_acel_plot, mpu_acel_plot -> lists[f], create_fnode(log_data[i][f + 3]));
	plot_add_value(mpu_magn_plot, mpu_magn_plot -> lists[f], create_fnode(log_data[i][f + 6]));
      }
      
      fprintf(mpu_log_file, "\n");

      graph_plot(mpu_gyro_plot);
      graph_plot(mpu_acel_plot);
      graph_plot(mpu_magn_plot);
      nano_sleep(100000000);
    }
    
    fflush(stdout);

    //graph_plot(mpu_gyro_plot);
    
    fclose(mpu_log_file);
  }
}

void initMPU9250() {
  
  // Initialize MPU9250 device
  // wake up device
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x00);   // Clear sleep mode bit (6), enable all sensors
  nano_sleep(100000000); // Delay 100 ms for PLL to get established on x-axis gyro; should check for PLL ready interrupt
  
  // get stable time source, setting source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x01);
  nano_sleep(200000000);
  
  // Configure Gyro and Accelerometer
  // Disable FSYNC and set accelerometer and gyro bandwidth to 44 and 42 Hz, respectively;
  // DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz for both
  // Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, CONFIG, 0x03);
  
  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, SMPLRT_DIV, 0x04);   // 200 Hz; same as rate in CONFIG above
  
  // Set gyroscope full scale range
  // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
  uint8_t c = i2cReadByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG);   // get GYRO_CONFIG register value
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x02; // Clear Fchoice bits [1:0]
  c = c & ~0x18; // Clear AFS bits [4:3]
  //~~~c = c | Gscale << 3; // Set full scale range for the gyro
  c = c | 0 << 3;//!?!?!?

  // c =| 0x00; // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of GYRO_CONFIG
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG, c);   // Write new GYRO_CONFIG value to register
  
  // Set accelerometer full-scale range configuration
  c = i2cReadByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG);   // get ACCEL_CONFIG register value
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x18;  // Clear AFS bits [4:3]
  //~~~c = c | Ascale << 3; // Set full scale range for the accelerometer
  c = c | 0 << 3;//!?!?!?

  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG, c);   // Wrute new ACCEL_CONFIG register value
  
  // Set accelerometer sample rate configuration
  // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
  // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
  c = i2cReadByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG2);   // get ACCEL_CONFIG 2 register value
  c = c & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
  c = c | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG2, c);   // Write new ACCEL_CONFIG 2 register value
  
  // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
  // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

  // Configure Interrupts and Bypass Enable
  // Set interrupt pin active high, push-pull, and clear on read of INT_STATUS, enable I2C_BYPASS_EN so additional chips
  // can join the I2C bus and all can be controlled by the Arduino as master
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_PIN_CFG, 0x22);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_ENABLE,  0x01);   // Enable data ready (bit 0) interrupt
  nano_sleep(100000000);
}

void resetMPU9250() {
  // reset device
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x80);   // Write a 1 to reset bit 7; toggle reset bit
  nano_sleep(100000000);
}


// Function which accumulates gyro and accelerometer data after device initialization. It calculates the average
// of the at-rest readings and then loads the resulting offsets into accelerometer and gyro bias registers.
void calibrateMPU9250(float * dest1, float * dest2) {
  
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii, packet_count, fifo_count;
  int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

  // reset device, reset all registers, clear gyro and accelerometer bias registers
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x80); // Write a 1 to reset bit 7; toggle reset device
  nano_sleep(100000000);

  // get stable time source
  // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x01);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_2, 0x00);
  nano_sleep(200000000);

  // Configure device for bias calculation
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_ENABLE,   0x00);   // Disable all interrupts
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN,      0x00);   // Disable FIFO
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1,   0x00);   // Turn on internal clock source
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, I2C_MST_CTRL, 0x00);   // Disable I2C master
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL,    0x00);   // Disable FIFO and I2C master modes
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL,    0x0C);   // Reset FIFO and DMP
  nano_sleep(15000000);

  // Configure MPU9250 gyro and accelerometer for bias calculation
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, CONFIG,       0x01);   // Set low-pass filter to 188 Hz
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, SMPLRT_DIV,   0x00);   // Set sample rate to 1 kHz
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG,  0x00);   // Set gyro full-scale to max 250 deg/second
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG, 0x00);   // Set accelerometer full-scale to max 2 g

  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

  // Configure FIFO to capture accelerometer and gyro data for bias calculation
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL, 0x40);   // Enable FIFO
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN,   0x78);   // Enable sensors for 512 byte FIFO
  nano_sleep(40000000); // accumulate 40 samples in 80 milliseconds = 480 bytes

  // At end of sample accumulation, turn off FIFO sensor read
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN, 0x00);   // Disable sensors for FIFO
  readBytes(i2c_device -> i2c -> i2c_address, FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
  fifo_count = ((uint16_t) data[0] << 8) | data[1];
  packet_count = fifo_count / 12;// How many sets of full gyro and accelerometer data for averaging

  for (ii = 0; ii < packet_count; ii++) {
    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    readBytes(i2c_device -> i2c -> i2c_address, FIFO_R_W, 12, &data[0]); // read data for averaging
    
    accel_temp[0] = (int16_t) (((int16_t) data[0]  << 8) | data[1] );  // Form signed 16-bit integer for each sample
    accel_temp[1] = (int16_t) (((int16_t) data[2]  << 8) | data[3] );
    accel_temp[2] = (int16_t) (((int16_t) data[4]  << 8) | data[5] );
    gyro_temp[0]  = (int16_t) (((int16_t) data[6]  << 8) | data[7] );
    gyro_temp[1]  = (int16_t) (((int16_t) data[8]  << 8) | data[9] );
    gyro_temp[2]  = (int16_t) (((int16_t) data[10] << 8) | data[11]);

    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[1] += (int32_t) accel_temp[1];
    accel_bias[2] += (int32_t) accel_temp[2];
    gyro_bias[0]  += (int32_t) gyro_temp[0];
    gyro_bias[1]  += (int32_t) gyro_temp[1];
    gyro_bias[2]  += (int32_t) gyro_temp[2];

  }
  accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
  accel_bias[1] /= (int32_t) packet_count;
  accel_bias[2] /= (int32_t) packet_count;
  gyro_bias[0]  /= (int32_t) packet_count;
  gyro_bias[1]  /= (int32_t) packet_count;
  gyro_bias[2]  /= (int32_t) packet_count;

  if (accel_bias[2] > 0L) accel_bias[2] -= (int32_t) accelsensitivity;  // Remove gravity from the z-axis accelerometer bias calculation
  else accel_bias[2] += (int32_t) accelsensitivity;

  // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
  data[0] = (-gyro_bias[0] / 4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
  data[1] = (-gyro_bias[0] / 4      ) & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
  data[2] = (-gyro_bias[1] / 4  >> 8) & 0xFF;
  data[3] = (-gyro_bias[1] / 4      ) & 0xFF;
  data[4] = (-gyro_bias[2] / 4  >> 8) & 0xFF;
  data[5] = (-gyro_bias[2] / 4      ) & 0xFF;

  dest1[0] = (float) gyro_bias[0] / (float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
  dest1[1] = (float) gyro_bias[1] / (float) gyrosensitivity;
  dest1[2] = (float) gyro_bias[2] / (float) gyrosensitivity;

  // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
  // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
  // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
  // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
  // the accelerometer biases calculated above must be divided by 8.

  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
  readBytes(i2c_device -> i2c -> i2c_address, XA_OFFSET_H, 2, &data[0]); // Read factory accelerometer trim values
  accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(i2c_device -> i2c -> i2c_address, YA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(i2c_device -> i2c -> i2c_address, ZA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];

  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

  for(ii = 0; ii < 3; ii++) {
    if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temp compensation bit is set, record that fact in mask_bit
  }

  // Construct total accelerometer bias, including calculated average accelerometer bias from above
  accel_bias_reg[0] -= (accel_bias[0] / 8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
  accel_bias_reg[1] -= (accel_bias[1] / 8);
  accel_bias_reg[2] -= (accel_bias[2] / 8);
  
  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
  data[1] = (accel_bias_reg[0])      & 0xFF;
  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
  data[3] = (accel_bias_reg[1])      & 0xFF;
  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
  data[5] = (accel_bias_reg[2])      & 0xFF;
  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

  // Output scaled accelerometer biases for manual subtraction in the main program
  dest2[0] = (float) accel_bias[0] / (float) accelsensitivity;
  dest2[1] = (float) accel_bias[1] / (float) accelsensitivity;
  dest2[2] = (float) accel_bias[2] / (float) accelsensitivity;
}

void initAK8963(float * destination) {
  
  // First extract the factory calibration for each magnetometer axis
  uint8_t rawData[3];  // x/y/z gyro calibration data stored here
  i2cWriteByteData(i2c_device -> i2c -> i2c_slave_address, AK8963_CNTL, 0x00); // Power down magnetometer
  nano_sleep(10000000);
  i2cWriteByteData(i2c_device -> i2c -> i2c_slave_address, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
  nano_sleep(10000000);
  
  // Read the x, y, and z-axis calibration values
  readBytes(i2c_device -> i2c -> i2c_slave_address, AK8963_ASAX, 3, &rawData[0]);
  destination[0] =  (float)(rawData[0] - 128)/256. + 1.;   // Return x-axis sensitivity adjustment values, etc.
  destination[1] =  (float)(rawData[1] - 128)/256. + 1.;
  destination[2] =  (float)(rawData[2] - 128)/256. + 1.;
  i2cWriteByteData(i2c_device -> i2c -> i2c_slave_address, AK8963_CNTL, 0x00); // Power down magnetometer
  nano_sleep(10000000);
  // Configure the magnetometer for continuous read and highest resolution
  // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
  // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
  // Set magnetometer data resolution and samle ODR
  i2cWriteByteData(i2c_device -> i2c -> i2c_slave_address, AK8963_CNTL, Mscale << 4 | Mmode);
  nano_sleep(10000000);
}

bool initialize_i2c(module * initialent) {
  i2c_device = initialent;
  initialent -> i2c = malloc(sizeof(I2C));
  initialent -> i2c -> i2c_address       = i2cOpen(1, MPU9250_ADDRESS, 0);
  initialent -> i2c -> i2c_slave_address = i2cOpen(1,  AK8963_ADDRESS, 0);
  
  // Graphics memory allocation
  mpu_gyro_plot = create_plot("    MPU Gyro Axes v.s. Time    ", 3);
  mpu_acel_plot = create_plot("MPU Acelerometer Axes v.s. Time", 3);
  mpu_magn_plot = create_plot("MPU Magnetometer Axes v.s. Time", 3);

  if (i2cReadByteData(initialent -> i2c -> i2c_address, 0) >= 0) {
    //i2c_device = initialent;

    // Set function pointers
    i2c_device -> i2c -> gyros = &readGyroData;
    i2c_device -> i2c -> temperature = &readTempData;
    i2c_device -> i2c -> accelerometers = &readAccelData;

    calibrateMPU9250(gyroBias, accelBias);
    initMPU9250();

    initAK8963(magCalibration);
    
    // Successful initialization, open log file for recording temperature data
    mpu_log_file = fopen(mpu_log_file_name, "a");
    fprintf(mpu_log_file, GREEN "\nRecording MPU Data\nTIME\tGyro x\tGyro y\tGyro z\tAcel x\tAcel y\tAcel z\tMagn x\tMagn y\tMagn z\n" RESET);
    fclose(mpu_log_file);

    // Spawn a logging thread
    mpu_termination_signal = false;
    pthread_create(&mpu_thread, NULL, log_mpu_data, NULL);
    
    return true;
  }
  return false;
}

void terminate_mpu_logging() {
  mpu_termination_signal = true;
}
