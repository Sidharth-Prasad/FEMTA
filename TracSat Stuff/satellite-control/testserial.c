#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


// UM7 Configuration Registers
//All of this are the values to setup the packets for parsing and understanding
#define CREG_COM_SETTINGS     0x00    // General communication settings
#define CREG_COM_RATES1       0x01    // Broadcast rate settings
#define CREG_COM_RATES2       0x02    // Broadcast rate settings
#define CREG_COM_RATES3       0x03    // Broadcast rate settings
#define CREG_COM_RATES4       0x04    // Broadcast rate settings
#define CREG_COM_RATES5       0x05    // Broadcast rate settings
#define CREG_COM_RATES6       0x06    // Broadcast rate settings
#define CREG_COM_RATES7       0x07    // Broadcast rate settings
#define CREG_MISC_SETTINGS    0x08    // Miscelaneous settings
#define CREG_GYRO_TRIM_X      0x0C    // Bias trim for x-axis rate gyro
#define CREG_GYRO_TRIM_Y      0x0D    // Bias trim for y-axis rate gyro
#define CREG_GYRO_TRIM_Z      0x0E    // Bias trim for z-axis rate gyro
#define CREG_MAG_CAL1_1       0x0F    // Row 1, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL1_2       0x10    // Row 1, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL1_3       0x11    // Row 1, Column 3 of magnetometer calibration matrix
#define CREG_MAG_CAL2_1       0x12    // Row 2, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL2_2       0x13    // Row 2, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL2_3       0x14    // Row 2, Column 3 of magnetometer calibration matrix
#define CREG_MAG_CAL3_1       0x15    // Row 3, Column 1 of magnetometer calibration matrix
#define CREG_MAG_CAL3_2       0x16    // Row 3, Column 2 of magnetometer calibration matrix
#define CREG_MAG_CAL3_3       0x17    // Row 3, Column 3 of magnetometer calibration matrix
#define CREG_MAG_BIAS_X       0x18    // Magnetometer x-axis bias
#define CREG_MAG_BIAS_Y       0x19    // Magnetometer y-axis bias
#define CREG_MAG_BIAS_Z       0x1A    // Magnetometer z-axis bias

// UM7 Data Registers
#define DREG_HEALTH           0x55    // Contains information about the health and status of the UM7
#define DREG_TEMPERATURE      0x5F    // Temperature data
#define DREG_TEMPERATURE_TIME 0x60    // Time at which temperature data was aquired
#define DREG_GYRO_PROC_X      0x61    // Processed x-axis rate gyro data
#define DREG_GYRO_PROC_Y      0x62    // Processed y-axis rate gyro data
#define DREG_GYRO_PROC_Z      0x63    // Processed z-axis rate gyro data
#define DREG_GYRO_PROC_TIME   0x64    // Time at which rate gyro data was aquired
#define DREG_ACCEL_PROC_X     0x65    // Processed x-axis accelerometer data
#define DREG_ACCEL_PROC_Y     0x66    // Processed y-axis accelerometer data
#define DREG_ACCEL_PROC_Z     0x67    // Processed z-axis accelerometer data
#define DREG_ACCEL_PROC_TIME  0x68    // Time at which accelerometer data was aquired
#define DREG_MAG_PROC_X       0x69    // Processed x-axis magnetometer data
#define DREG_MAG_PROC_Y       0x6A    // Processed y-axis magnetometer data
#define DREG_MAG_PROC_Z       0x6B    // Processed z-axis magnetometer data
#define DREG_MAG_PROC_TIME    0x6C    // Time at which magnetometer data was aquired
#define DREG_QUAT_AB          0x6D    // Quaternion elements A and B
#define DREG_QUAT_CD          0x6E    // Quaternion elements C and D
#define DREG_QUAT_TIME        0x6F    // Time at which the sensor was at the specified quaternion rotation
#define DREG_GYRO_BIAS_X      0x89    // Gyro x-axis bias estimate
#define DREG_GYRO_BIAS_Y      0x8A    // Gyro y-axis bias estimate
#define DREG_GYRO_BIAS_Z      0x8B    // Gyro z-axis bias estimate

// UM7 Commands
#define GET_FW_REVISION       0xAA    // Asks for the firmware revision
#define FLASH_COMMIT          0xAB    // Writes configuration settings to flash
#define RESET_TO_FACTORY      0xAC    // Reset all settings to factory defaults
#define ZERO_GYROS            0xAD    // Causes the rate gyro biases to be calibrated
#define SET_HOME_POSITION     0xAE    // Sets the current GPS location as the origin
#define SET_MAG_REFERENCE     0xB0    // Sets the magnetometer reference vector
#define RESET_EKF             0xB3    // Resets the EKF

// UM7 Batch addresses
#define BATCH_PROCESSED       0x61    // Processed gyro, accel, and magn batch start address
#define BATCH_QUATERNION      0x6D    // Quaternion batch start address
#define BATCH_EULER           0x70    // Euler batch batch start address

// Structure for holding received packet information
typedef struct UM7_packet_struct
{
    uint8_t Address;
    uint8_t PT;
    uint16_t Checksum;
    
    uint8_t data_length;
    uint8_tdata[30];
} UM7_packet;

// parses data in rx_data w/length rx_length to find packet in data
// if packet found structure packet is filled with packet data
// if not enough data parse returns 1
// if enough data but o header returns 2
// if header found but insufficient data to parse whole packet then returns 3
// if received but checksum was bad then 4
// if good packet received, fills parse_serial and returns 0


uint8_t parse_serial_data(uint8_t* rx_data, uint8_t rx_length, UM7_packet* packet)
{
    uint8_t index;

    //ensure data buffer big enough to have full packet (min is 7)
    if(rx_length < 7)
    {
        return 1;
    }

    //Try to find the snp start sequence for the packet
    for(index = 0; index < (rx_length-2);index++)
    {
        if(rx_data[index]=='s' && rx_data[index+1]=='n' && rx_data[index+2]==
        'p')
        {
            break;
        }
    }

    uint8_t packet_index = index;

    if(packet_index == (rx_length - 2))
    {
        return 2;
    }

    if((rx_length - packet_index)<7)
    {
        return 3;
    }

    uint8_t PT = rx_data[packet_index + 3];

    uint8_t packet_has_data = (PT >> 7) & 0x01;     // Check bit 7 (HAS_DATA)
    uint8_t packet_is_batch = (PT >> 6) & 0x01;      // Check bit 6 (IS_BATCH)
    uint8_t batch_length = (PT >> 2) & 0x0F; 

    uint8_t data_length = 0;

    if(packet_has_data)
    {
        if(packet_is_batch)
        {
            data_length = 4*batch_length;
        }

        else
        {
            data_length = 4;
        }
    }

    else
    {
        data_length = 0;
    }

    if((rx_length - packet_index)<(data_length + 5))
    {
        return 3;
    }
    
    packet -> Address = rx_data[packet_index + 4];
    packet -> PT = PT;

    packet->data_length = data_length;
    uint16_t computed_checksum = ‘s’ + ‘n’ + ‘p’ + packet_data->PT + packet_data->Address;
    for( index = 0; index < data_length; index++)
    {
        packet -> data[index]=rx_data[packet_index + 5 + index];

        computed_checksum += packet->data[index];
    } 

    uint16_t received_checksum = (rx_data[packet_index + 5 + data_length] << 8);

    received_checksum |= rx_data[packet_index + 6 + data_length];

    if(received_checksum != computed_checksum)
    {
        return 4;
    }

    return 0;
}

//TODO:
//Acessing desired address
//Checking Data length
//Pulling out packet's data array

int main( int argc, const char* argv[] )
{
	bool serial_sucess = intitialize_serial();
}



