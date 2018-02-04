
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <pigpio.h>
#include <stdbool.h>

#include "unified-controller.h"
#include "i2c-interface.h"

#define SMPLRT_DIV       0x19
#define CONFIG           0x1A
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define TEMP_OUT_H       0x41
#define TEMP_OUT_L       0x42
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B  // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define MPU9250_ADDRESS  0x68
#define FIFO_COUNTH      0x72
#define FIFO_R_W         0x74
#define XA_OFFSET_H      0x77
#define YA_OFFSET_H      0x7A
#define ZA_OFFSET_H      0x7D



float readTempData() {
  uint8_t low_bits  = i2cReadByteData(i2c_device -> i2c -> i2c_address, TEMP_OUT_L);
  uint8_t high_bits = i2cReadByteData(i2c_device -> i2c -> i2c_address, TEMP_OUT_H);
  int16_t bytes = (int16_t) (((int16_t) high_bits) << 8 | low_bits);  // Create 2-byte value
  return ((float) bytes) / 333.87 + 21.0; //  Convert to Centigrade
}


void nano_sleep(long duration) {
  struct timespec delay, result;
  delay.tv_sec = 0;
  delay.tv_nsec = duration;
  nanosleep(&delay, &result);
}

void initMPU9250() {
  
  // Initialize MPU9250 device
  // wake up device
  //~~~writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00); // Clear sleep mode bit (6), enable all sensors
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x00);
  nano_sleep(100000000); // Delay 100 ms for PLL to get established on x-axis gyro; should check for PLL ready interrupt
  
  // get stable time source, setting source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
  //~~~writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01); 
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x01);
  
  // Configure Gyro and Accelerometer
  // Disable FSYNC and set accelerometer and gyro bandwidth to 44 and 42 Hz, respectively;
  // DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz for both
  // Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
  //~~~writeByte(MPU9250_ADDRESS, CONFIG, 0x03);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, CONFIG, 0x03);
  
  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  //~~~writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x04);  // Use a 200 Hz rate; the same rate set in CONFIG above
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, SMPLRT_DIV, 0x04);
  
  // Set gyroscope full scale range
  // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
  //~~~uint8_t c = readByte(MPU9250_ADDRESS, GYRO_CONFIG); // get current GYRO_CONFIG register value
  uint8_t c = i2cReadByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG);
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x02; // Clear Fchoice bits [1:0]
  c = c & ~0x18; // Clear AFS bits [4:3]
  //~~~c = c | Gscale << 3; // Set full scale range for the gyro
  c = c | 0 << 3;//!?!?!?

  // c =| 0x00; // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of GYRO_CONFIG
  //~~~writeByte(MPU9250_ADDRESS, GYRO_CONFIG, c ); // Write new GYRO_CONFIG value to register
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG, c);
  
  // Set accelerometer full-scale range configuration
  //~~~c = readByte(MPU9250_ADDRESS, ACCEL_CONFIG); // get current ACCEL_CONFIG register value
  c = i2cReadByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG);
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x18;  // Clear AFS bits [4:3]
  //~~~c = c | Ascale << 3; // Set full scale range for the accelerometer
  c = c | 0 << 3;//!?!?!?

  //~~~writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, c); // Write new ACCEL_CONFIG register value
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG, c);
  
  // Set accelerometer sample rate configuration
  // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
  // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
  //~~~c = readByte(MPU9250_ADDRESS, ACCEL_CONFIG2); // get current ACCEL_CONFIG2 register value
  c = i2cReadByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG2);
  c = c & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
  c = c | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
  //~~~writeByte(MPU9250_ADDRESS, ACCEL_CONFIG2, c); // Write new ACCEL_CONFIG2 register value
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG2, c);
  
  // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
  // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

  // Configure Interrupts and Bypass Enable
  // Set interrupt pin active high, push-pull, and clear on read of INT_STATUS, enable I2C_BYPASS_EN so additional chips
  // can join the I2C bus and all can be controlled by the Arduino as master
  //~~~writeByte(MPU9250_ADDRESS, INT_PIN_CFG, 0x22);
  //~~~writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_PIN_CFG, 0x22);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_ENABLE, 0x01);
}

void resetMPU9250() {
  // reset device
  //~~~writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x80);
  nano_sleep(100000000);
}

/*
void calibrateMPU9250(float * dest1, float * dest2)
{
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii, packet_count, fifo_count;
  int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

  // reset device, reset all registers, clear gyro and accelerometer bias registers
  //~~~writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x80);
  nano_sleep(100000000);

  // get stable time source
  // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
  //writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);
  //writeByte(MPU9250_ADDRESS, PWR_MGMT_2, 0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1, 0x01);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_2, 0x00);
  nano_sleep(200000000);

  // Configure device for bias calculation
  //~~~writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x00);   // Disable all interrupts
  //~~~writeByte(MPU9250_ADDRESS, FIFO_EN, 0x00);      // Disable FIFO
  //~~~writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);   // Turn on internal clock source
  //~~~writeByte(MPU9250_ADDRESS, I2C_MST_CTRL, 0x00); // Disable I2C master
  //~~~writeByte(MPU9250_ADDRESS, USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
  //~~~writeByte(MPU9250_ADDRESS, USER_CTRL, 0x0C);    // Reset FIFO and DMP
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, INT_ENABLE,   0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN,      0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, PWR_MGMT_1,   0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, I2C_MST_CTRL, 0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL,    0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL,    0x0C);
  nano_sleep(15000000);

  // Configure MPU9250 gyro and accelerometer for bias calculation
  //~~~writeByte(MPU9250_ADDRESS, CONFIG, 0x01);      // Set low-pass filter to 188 Hz
  //~~~writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
  //~~~writeByte(MPU9250_ADDRESS, GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
  //~~~writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, CONFIG,       0x01);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, SMPLRT_DIV,   0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, GYRO_CONFIG,  0x00);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, ACCEL_CONFIG, 0x00);

  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

  // Configure FIFO to capture accelerometer and gyro data for bias calculation
  //~~~writeByte(MPU9250_ADDRESS, USER_CTRL, 0x40);   // Enable FIFO
  //~~~writeByte(MPU9250_ADDRESS, FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO (max size 512 bytes in MPU-9250)
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, USER_CTRL, 0x40);
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN,   0x78);
  nano_sleep(40000000); // accumulate 40 samples in 80 milliseconds = 480 bytes

  // At end of sample accumulation, turn off FIFO sensor read
  //~~writeByte(MPU9250_ADDRESS, FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
  i2cWriteByteData(i2c_device -> i2c -> i2c_address, FIFO_EN, 0x00);
  readBytes(MPU9250_ADDRESS, FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
  fifo_count = ((uint16_t)data[0] << 8) | data[1];
  packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

  for (ii = 0; ii < packet_count; ii++) {
    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    readBytes(MPU9250_ADDRESS, FIFO_R_W, 12, &data[0]); // read data for averaging
    accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
    accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;
    gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
    gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
    gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;

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

  if (accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
  else {accel_bias[2] += (int32_t) accelsensitivity;}

  // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
  data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
  data[3] = (-gyro_bias[1]/4)       & 0xFF;
  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
  data[5] = (-gyro_bias[2]/4)       & 0xFF;

  dest1[0] = (float) gyro_bias[0]/(float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
  dest1[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
  dest1[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

  // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
  // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
  // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
  // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
  // the accelerometer biases calculated above must be divided by 8.

  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
  readBytes(MPU9250_ADDRESS, XA_OFFSET_H, 2, &data[0]); // Read factory accelerometer trim values
  accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(MPU9250_ADDRESS, YA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(MPU9250_ADDRESS, ZA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];

  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

  for(ii = 0; ii < 3; ii++) {
    if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
  }

  // Construct total accelerometer bias, including calculated average accelerometer bias from above
  accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
  accel_bias_reg[1] -= (accel_bias[1]/8);
  accel_bias_reg[2] -= (accel_bias[2]/8);
  
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
  dest2[0] = (float)accel_bias[0]/(float)accelsensitivity;
  dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
  dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;
}
*/


bool initialize_i2c(module * initialent) {
  initialent -> i2c = malloc(sizeof(I2C));
  initialent -> i2c -> i2c_address = i2cOpen(1, MPU9250_ADDRESS, 0);
  if (i2cReadByteData(initialent -> i2c -> i2c_address, 0) >= 0) {
    i2c_device = initialent;


    // Set the temperature function pointer
    i2c_device -> i2c -> temperature = &readTempData;


    resetMPU9250();
    initMPU9250();
    return true;
  }
  return false;
}
