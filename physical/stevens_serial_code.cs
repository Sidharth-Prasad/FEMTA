using System;
using System.IO.Ports;
using System.Threading.Tasks;
using System.Collections.Generic;

using static System.Math;

using FemtaMath;
using DigitalPi;

namespace InertialMeasurement
{
	///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
	///   Namespace:      <InertialMeasurement>                                                    ///
	///   Class:          <BNO055>                                                                 ///
	///   Description:    'A class for communicating over a UART serial port with the BNO055       ///
	///                    inertial measurement sensor.'                                           ///
	///                                                                                            ///
	///   Author:         <Steven_Pugia>                                                           ///
	///   Notes:          <None>                                                                   ///
	///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
	public class BNO055
	{
		private const byte BNO055_ADDRESS_A                     = 0x28;
		private const byte BNO055_ADDRESS_B                     = 0x29;
		private const byte BNO055_ID                            = 0xA0;

		private const byte BNO055_PAGE_ID_ADDR                  = 0X07;

		private const byte BNO055_CHIP_ID_ADDR                  = 0x00;
		private const byte BNO055_ACCEL_REV_ID_ADDR             = 0x01;
		private const byte BNO055_MAG_REV_ID_ADDR               = 0x02;
		private const byte BNO055_IMU_REV_ID_ADDR               = 0x03;
		private const byte BNO055_SW_REV_ID_LSB_ADDR            = 0x04;
		private const byte BNO055_SW_REV_ID_MSB_ADDR            = 0x05;
		private const byte BNO055_BL_REV_ID_ADDR                = 0X06;

		private const byte BNO055_ACCEL_DATA_X_LSB_ADDR         = 0X08;
		private const byte BNO055_ACCEL_DATA_X_MSB_ADDR         = 0X09;
		private const byte BNO055_ACCEL_DATA_Y_LSB_ADDR         = 0X0A;
		private const byte BNO055_ACCEL_DATA_Y_MSB_ADDR         = 0X0B;
		private const byte BNO055_ACCEL_DATA_Z_LSB_ADDR         = 0X0C;
		private const byte BNO055_ACCEL_DATA_Z_MSB_ADDR         = 0X0D;

		private const byte BNO055_MAG_DATA_X_LSB_ADDR           = 0X0E;
		private const byte BNO055_MAG_DATA_X_MSB_ADDR           = 0X0F;
		private const byte BNO055_MAG_DATA_Y_LSB_ADDR           = 0X10;
		private const byte BNO055_MAG_DATA_Y_MSB_ADDR           = 0X11;
		private const byte BNO055_MAG_DATA_Z_LSB_ADDR           = 0X12;
		private const byte BNO055_MAG_DATA_Z_MSB_ADDR           = 0X13;

		private const byte BNO055_GYRO_DATA_X_LSB_ADDR          = 0X14;
		private const byte BNO055_GYRO_DATA_X_MSB_ADDR          = 0X15;
		private const byte BNO055_GYRO_DATA_Y_LSB_ADDR          = 0X16;
		private const byte BNO055_GYRO_DATA_Y_MSB_ADDR          = 0X17;
		private const byte BNO055_GYRO_DATA_Z_LSB_ADDR          = 0X18;
		private const byte BNO055_GYRO_DATA_Z_MSB_ADDR          = 0X19;

		private const byte BNO055_EULER_H_LSB_ADDR              = 0X1A;
		private const byte BNO055_EULER_H_MSB_ADDR              = 0X1B;
		private const byte BNO055_EULER_R_LSB_ADDR              = 0X1C;
		private const byte BNO055_EULER_R_MSB_ADDR              = 0X1D;
		private const byte BNO055_EULER_P_LSB_ADDR              = 0X1E;
		private const byte BNO055_EULER_P_MSB_ADDR              = 0X1F;

		private const byte BNO055_QUATERNION_DATA_W_LSB_ADDR    = 0X20;
		private const byte BNO055_QUATERNION_DATA_W_MSB_ADDR    = 0X21;
		private const byte BNO055_QUATERNION_DATA_X_LSB_ADDR    = 0X22;
		private const byte BNO055_QUATERNION_DATA_X_MSB_ADDR    = 0X23;
		private const byte BNO055_QUATERNION_DATA_Y_LSB_ADDR    = 0X24;
		private const byte BNO055_QUATERNION_DATA_Y_MSB_ADDR    = 0X25;
		private const byte BNO055_QUATERNION_DATA_Z_LSB_ADDR    = 0X26;
		private const byte BNO055_QUATERNION_DATA_Z_MSB_ADDR    = 0X27;

		private const byte BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR  = 0X28;
		private const byte BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR  = 0X29;
		private const byte BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR  = 0X2A;
		private const byte BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR  = 0X2B;
		private const byte BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR  = 0X2C;
		private const byte BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR  = 0X2D;

		private const byte BNO055_GRAVITY_DATA_X_LSB_ADDR       = 0X2E;
		private const byte BNO055_GRAVITY_DATA_X_MSB_ADDR       = 0X2F;
		private const byte BNO055_GRAVITY_DATA_Y_LSB_ADDR       = 0X30;
		private const byte BNO055_GRAVITY_DATA_Y_MSB_ADDR       = 0X31;
		private const byte BNO055_GRAVITY_DATA_Z_LSB_ADDR       = 0X32;
		private const byte BNO055_GRAVITY_DATA_Z_MSB_ADDR       = 0X33;

		private const byte BNO055_TEMP_ADDR                     = 0X34;

		private const byte BNO055_CALIB_STAT_ADDR               = 0X35;
		private const byte BNO055_SELFTEST_RESULT_ADDR          = 0X36;
		private const byte BNO055_INTR_STAT_ADDR                = 0X37;

		private const byte BNO055_SYS_CLK_STAT_ADDR             = 0X38;
		private const byte BNO055_SYS_STAT_ADDR                 = 0X39;
		private const byte BNO055_SYS_ERR_ADDR                  = 0X3A;

		private const byte BNO055_UNIT_SEL_ADDR                 = 0X3B;
		private const byte BNO055_DATA_SELECT_ADDR              = 0X3C;

		private const byte BNO055_OPR_MODE_ADDR                 = 0X3D;
		private const byte BNO055_PWR_MODE_ADDR                 = 0X3E;

		private const byte BNO055_SYS_TRIGGER_ADDR              = 0X3F;
		private const byte BNO055_TEMP_SOURCE_ADDR              = 0X40;

		private const byte BNO055_AXIS_MAP_CONFIG_ADDR          = 0X41;
		private const byte BNO055_AXIS_MAP_SIGN_ADDR            = 0X42;

		private const byte AXIS_REMAP_X                         = 0x00;
		private const byte AXIS_REMAP_Y                         = 0x01;
		private const byte AXIS_REMAP_Z                         = 0x02;
		private const byte AXIS_REMAP_POSITIVE                  = 0x00;
		private const byte AXIS_REMAP_NEGATIVE                  = 0x01;

		private const byte BNO055_SIC_MATRIX_0_LSB_ADDR         = 0X43;
		private const byte BNO055_SIC_MATRIX_0_MSB_ADDR         = 0X44;
		private const byte BNO055_SIC_MATRIX_1_LSB_ADDR         = 0X45;
		private const byte BNO055_SIC_MATRIX_1_MSB_ADDR         = 0X46;
		private const byte BNO055_SIC_MATRIX_2_LSB_ADDR         = 0X47;
		private const byte BNO055_SIC_MATRIX_2_MSB_ADDR         = 0X48;
		private const byte BNO055_SIC_MATRIX_3_LSB_ADDR         = 0X49;
		private const byte BNO055_SIC_MATRIX_3_MSB_ADDR         = 0X4A;
		private const byte BNO055_SIC_MATRIX_4_LSB_ADDR         = 0X4B;
		private const byte BNO055_SIC_MATRIX_4_MSB_ADDR         = 0X4C;
		private const byte BNO055_SIC_MATRIX_5_LSB_ADDR         = 0X4D;
		private const byte BNO055_SIC_MATRIX_5_MSB_ADDR         = 0X4E;
		private const byte BNO055_SIC_MATRIX_6_LSB_ADDR         = 0X4F;
		private const byte BNO055_SIC_MATRIX_6_MSB_ADDR         = 0X50;
		private const byte BNO055_SIC_MATRIX_7_LSB_ADDR         = 0X51;
		private const byte BNO055_SIC_MATRIX_7_MSB_ADDR         = 0X52;
		private const byte BNO055_SIC_MATRIX_8_LSB_ADDR         = 0X53;
		private const byte BNO055_SIC_MATRIX_8_MSB_ADDR         = 0X54;

		private const byte ACCEL_OFFSET_X_LSB_ADDR              = 0X55;
		private const byte ACCEL_OFFSET_X_MSB_ADDR              = 0X56;
		private const byte ACCEL_OFFSET_Y_LSB_ADDR              = 0X57;
		private const byte ACCEL_OFFSET_Y_MSB_ADDR              = 0X58;
		private const byte ACCEL_OFFSET_Z_LSB_ADDR              = 0X59;
		private const byte ACCEL_OFFSET_Z_MSB_ADDR              = 0X5A;

		private const byte MAG_OFFSET_X_LSB_ADDR                = 0X5B;
		private const byte MAG_OFFSET_X_MSB_ADDR                = 0X5C;
		private const byte MAG_OFFSET_Y_LSB_ADDR                = 0X5D;
		private const byte MAG_OFFSET_Y_MSB_ADDR                = 0X5E;
		private const byte MAG_OFFSET_Z_LSB_ADDR                = 0X5F;
		private const byte MAG_OFFSET_Z_MSB_ADDR                = 0X60;

		private const byte IMU_OFFSET_X_LSB_ADDR               	= 0X61;
		private const byte IMU_OFFSET_X_MSB_ADDR               	= 0X62;
		private const byte IMU_OFFSET_Y_LSB_ADDR               	= 0X63;
		private const byte IMU_OFFSET_Y_MSB_ADDR               	= 0X64;
		private const byte IMU_OFFSET_Z_LSB_ADDR               	= 0X65;
		private const byte IMU_OFFSET_Z_MSB_ADDR               	= 0X66;

		private const byte ACCEL_RADIUS_LSB_ADDR                = 0X67;
		private const byte ACCEL_RADIUS_MSB_ADDR                = 0X68;
		private const byte MAG_RADIUS_LSB_ADDR                  = 0X69;
		private const byte MAG_RADIUS_MSB_ADDR                  = 0X6A;

		private const byte POWER_MODE_NORMAL                    = 0X00;
		private const byte POWER_MODE_LOWPOWER                  = 0X01;
		private const byte POWER_MODE_SUSPEND                   = 0X02;

		private const byte OPERATION_MODE_CONFIG                = 0X00;
		private const byte OPERATION_MODE_ACCONLY               = 0X01;
		private const byte OPERATION_MODE_MAGONLY               = 0X02;
		private const byte OPERATION_MODE_IMUNLY                = 0X03;
		private const byte OPERATION_MODE_ACCMAG                = 0X04;
		private const byte OPERATION_MODE_ACCIMU                = 0X05;
		private const byte OPERATION_MODE_MAGIMU                = 0X06;
		private const byte OPERATION_MODE_AMG                   = 0X07;
		private const byte OPERATION_MODE_IMUPLUS               = 0X08;
		private const byte OPERATION_MODE_COMPASS               = 0X09;
		private const byte OPERATION_MODE_M4G                   = 0X0A;
		private const byte OPERATION_MODE_NDOF_FMC_OFF          = 0X0B;
		private const byte OPERATION_MODE_NDOF                  = 0X0C;

		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		///   Namespace:      <InertialMeasurement>                                                    ///
		///   Class:          <SystemStatus>                                                           ///
		///   Description:    'A class that contains system status information from the BNO055         ///
		///                    sensor.'                                                                ///
		///                                                                                            ///
		///   Author:         <Steven_Pugia>                                                           ///
		///   Notes:          <None>                                                                   ///
		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		public class SystemStatus
		{
			public byte StatusReg 		{ get; set; }
			public byte SelfTestResult 	{ get; set; }
			public byte ErrorReg 		{ get; set; }
		}

		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		///   Namespace:      <InertialMeasurement>                                                    ///
		///   Class:          <Revisions>                                                              ///
		///   Description:    'A class that contains sensor revision information for the BNO055        ///
		///                    sensor.'                                                                ///
		///                                                                                            ///
		///   Author:         <Steven_Pugia>                                                           ///
		///   Notes:          <None>                                                                   ///
		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		public class Revisions
		{
			public byte Software 		{ get; set; }
			public byte Bootloader 		{ get; set; }
			public byte AccelID 		{ get; set; }
			public byte MagID 			{ get; set; }
			public byte IMUID 			{ get; set; }
		}

		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		///   Namespace:      <InertialMeasurement>                                                    ///
		///   Class:          <CalibrationStatus>                                                      ///
		///   Description:    'A class that contains calibration information from the BNO055           ///
		///                    sensor.'                                                                ///
		///                                                                                            ///
		///   Author:         <Steven_Pugia>                                                           ///
		///   Notes:          <None>                                                                   ///
		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		public class CalibrationStatus
		{
			public byte System 			{ get; set; }
			public byte IMU 			{ get; set; }
			public byte Accel 			{ get; set; }
			public byte Mag 			{ get; set; }
		}

		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		///   Namespace:      <InertialMeasurement>                                                    ///
		///   Class:          <AxisRemap>                                                              ///
		///   Description:    'A class that contains axis remap information from the BNO055            ///
		///                    sensor.'                                                                ///
		///                                                                                            ///
		///   Author:         <Steven_Pugia>                                                           ///
		///   Notes:          <None>                                                                   ///
		///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-///
		public class AxisRemap
		{
			public byte X 				{ get; set; }
			public byte Y 				{ get; set; }
			public byte Z 				{ get; set; }
			public byte XSign 			{ get; set; }
			public byte YSign 			{ get; set; }
			public byte ZSign 			{ get; set; }
		}

		private string PortName = @"/dev/serial0";

		private SerialPort Port;

		bool localConnected = false;
		public bool Connected
		{
			get { return localConnected; }
			set { localConnected = value; }
		}

		public bool Calibrated = false;

		public GPIO.GPIOPins ResetPin = GPIO.GPIOPins.GPIO_18;

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <BNO055>
		///   Description:    'This is the constructor for the BNO055 class.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public BNO055()
		{
			Initialize();
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <Initialize>
		///   Description:    'This method connects to, initializes and calibrates the BNO055.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private async void Initialize()
		{
			while (!Connected || !Calibrated)
			{
				GPIO.Write(ResetPin, true);
				Task.Delay(650).Wait();

				Port                = new SerialPort();
				Port.PortName       = PortName;
				Port.BaudRate       = 115200;
				Port.Parity         = Parity.None;
				Port.DataBits       = 8;
				Port.StopBits       = StopBits.One;
				Port.ReadTimeout    = 5000;
				Port.WriteTimeout   = 5000;

				try
				{
					//Port.Open();
					//while (!Port.IsOpen) { Task.Delay(1).Wait(); }
				}
				catch (Exception)
				{
					Console.WriteLine("Unable to open serial port.");
				}

				GPIO.Write(ResetPin, false);
				Task.Delay(10).Wait();
				GPIO.Write(ResetPin, true);
				Task.Delay(650).Wait();

				if (true) //(Port.IsOpen)
				{
					try
					{
						WriteByte(BNO055_PAGE_ID_ADDR, 0, false);
					}
					catch (Exception) { }

					//Port.DiscardInBuffer();

					SetMode(OPERATION_MODE_CONFIG);
					Task.Delay(30).Wait();

					WriteByte(BNO055_PAGE_ID_ADDR, 0);

					byte BNOID = BNO055_ID; //ReadBytes(BNO055_CHIP_ID_ADDR, 1)[0];

					if (BNO055_ID == BNOID)
					{
						GPIO.Write(ResetPin, false);
						Task.Delay(10).Wait();
						GPIO.Write(ResetPin, true);
						Task.Delay(650).Wait();

						WriteByte(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
						WriteByte(BNO055_SYS_TRIGGER_ADDR, 0x00);

						SetMode(OPERATION_MODE_NDOF);
						Task.Delay(30).Wait();

						Connected = true;
						Console.WriteLine("Connected to IMU.");

						Console.WriteLine("Calibrating IMU. . .");

						BNO055.CalibrationStatus lastCal;
						BNO055.CalibrationStatus cal = await Task.Run(() => GetCalibrationStatus());

						Console.WriteLine("System: " + cal.System.ToString() + ", Magnetometer: " + cal.Mag.ToString() + ", Gyroscope: " + cal.IMU.ToString() + ", Accelerometer: " + cal.Accel.ToString());
						while (cal.System < 2 || cal.Mag < 2 || cal.IMU < 2 || cal.Accel < 1)
						{
							lastCal = cal;
							cal = await Task.Run(() => GetCalibrationStatus());

							if (cal.Accel != lastCal.Accel || cal.IMU != lastCal.IMU || cal.Mag != lastCal.Mag || cal.System != lastCal.System)
							{
								Console.WriteLine("System: " + cal.System.ToString() + ", Magnetometer: " + cal.Mag.ToString() + ", Gyroscope: " + cal.IMU.ToString() + ", Accelerometer: " + cal.Accel.ToString());
							}
						}

						Calibrated = true;
						Console.WriteLine("IMU Calibrated.");
					}
					else
					{
						Console.WriteLine("Error: Chip ID mismatch.");
					}
				}

				if (!Connected || !Calibrated) //(!Port.IsOpen || !Connected || !Calibrated)
				{
					Console.WriteLine("Unable to connect to IMU. Trying again in 3 seconds. . .\n");

					Port.Close();
					Port.Dispose();
					Port = null;

					Task.Delay(3000).Wait();
				}
			}
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <SetMode>
		///   Description:    'This method sets the operation mode of the BNO055.'
		///
		///   Inputs:         (byte) mode
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private void SetMode(byte mode)
		{
			WriteByte(BNO055_OPR_MODE_ADDR, (byte)(mode & 0xFF), true);
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <Suspend>
		///   Description:    'This method suspends the sensor to conserve power.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public void Suspend()
		{
			try
			{
				WriteByte(BNO055_PWR_MODE_ADDR, POWER_MODE_SUSPEND);
			}
			catch (Exception)
			{
				Console.WriteLine("Unable to suspend IMU.");
			}
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <Resume>
		///   Description:    'This method resumes normal operation of the sensor.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public void Resume()
		{
			try
			{
				WriteByte(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);	
			}
			catch (Exception)
			{
				Console.WriteLine("Unable to activate IMU.");
			}
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <WriteByte>
		///   Description:    'This method writes a single byte to a register address over serial 
		///                    port.'
		///
		///   Inputs:         (byte) address, (byte) data, (bool) ack
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private void WriteByte(byte address, byte data, bool ack = true)
		{
			int maxAttempt  = 10;
			int attempt     = 0;

			while (attempt <= maxAttempt)
			{
				/*byte[] command = new byte[]
				{
					0xAA,
					0x00,
					address,
					(byte)1,
					data
				};*/

				//debug
				return;

				/*Port.DiscardInBuffer();
				Port.DiscardOutBuffer();

				Port.Write(command, 0, command.Length);

				if (!ack) { return; }
					
				byte[] response = new byte[2];
				while (Port.BytesToRead < 2) { }

				Port.Read(response, 0, 2);

				if (!(response[0] == 0xEE && response[1] == 0x07))
				{
					return;
				}

				attempt++;*/
			}

			Console.WriteLine("Write Error: Maximum attempts exceeded.");
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadBytes>
		///   Description:    'This method reads a number of bytes from a sensor register address
		///                    over serial port.'
		///
		///   Inputs:         (byte) address, (int) length
		///   Output:         byte[]
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private byte[] ReadBytes(byte address, int length)
		{
			int maxAttempt = 5;
			int attempt = 0;

			while (attempt <= maxAttempt)
			{
				/*byte[] command = new byte[]
				{
					0xAA,
					0x01,
					address,
					(byte)length
				};*/

				//debug
				return new byte[4];

				/*try
				{
					Port.DiscardInBuffer();
					Port.DiscardOutBuffer();

					Port.Write(command, 0, command.Length);

					byte[] response = new byte[2];
					while (Port.BytesToRead < 2) { }

					Port.Read(response, 0, 2);

					if (!(response[0] == 0xEE && response[1] == 0x07))
					{
						if (response[0] == 0xBB && response[1] == length)
						{
							byte[] result = new byte[length];
							while (Port.BytesToRead < length) { }

							Port.Read(result, 0, length);

							return result;
						}
					}
				}
				catch { }

				attempt++;*/
			}

			Console.WriteLine("Read Error: Maximum attempts exceeded");

			return new byte[length];
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <BNO055>
		///   Description:    'This method is the deconstructor for the BNO055 class.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		~ BNO055()
		{
			DisconnectFromBNO055();
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <DisconnectFromBNO055>
		///   Description:    'This method disconnects from the serial port.'
		///
		///   Inputs:         void
		///   Output:         void
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private void DisconnectFromBNO055()
		{
			if (Port != null)// && Port.IsOpen)
			{
				GPIO.Write(ResetPin, false);

				//Port.Close();
				Port = null;
			}
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <GetStatus>
		///   Description:    'This method gets the system status of the sensor.'
		///
		///   Inputs:         (bool) runSelfTest
		///   Output:         SystemStatus
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public SystemStatus GetStatus(bool runSelfTest)
		{
			byte selfTest = new byte();
			if (runSelfTest)
			{
				SetMode(OPERATION_MODE_CONFIG);
				Task.Delay(30).Wait();

				byte sysTrigger = ReadBytes(BNO055_SYS_TRIGGER_ADDR, 1)[0];
				WriteByte(BNO055_SYS_TRIGGER_ADDR, (byte)(sysTrigger | 0x01), true);

				Task.Delay(1000).Wait();

				selfTest = ReadBytes(BNO055_SELFTEST_RESULT_ADDR, 1)[0];

				SetMode(OPERATION_MODE_NDOF);
				Task.Delay(30).Wait();
			}

			SystemStatus status     = new SystemStatus();
			status.SelfTestResult   = selfTest;
			status.StatusReg        = ReadBytes(BNO055_SYS_STAT_ADDR, 1)[0];
			status.ErrorReg         = ReadBytes(BNO055_SYS_ERR_ADDR, 1)[0];

			return status;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <GetCalibrationStatus>
		///   Description:    'This method gets the calibration status of the sensor.'
		///
		///   Inputs:         void
		///   Output:         CalibrationStatus
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public CalibrationStatus GetCalibrationStatus()
		{
			//byte calStatus = ReadBytes(BNO055_CALIB_STAT_ADDR, 1)[0];

			CalibrationStatus cal   = new CalibrationStatus();
			cal.System              = 3; //(byte)((calStatus >> 6) & 0x03);
			cal.IMU                 = 3; //(byte)((calStatus >> 4) & 0x03);
			cal.Accel               = 3; //(byte)((calStatus >> 2) & 0x03);
			cal.Mag                 = 3; //(byte)(calStatus & 0x03);

			return cal;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadList>
		///   Description:    'This method reads a list array from a given register.'
		///
		///   Inputs:         (byte) address, (int) length
		///   Output:         List<short>
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private List<short> ReadList(byte address, int length)
		{
			List<short> vector 	= new List<short>();
			byte[] readBuffer 	= new byte[length * 2];

			readBuffer = ReadBytes(address, length * 2);

			for (int i = 0; i < length * 2; i = i + 2)
			{
				int entry = ((readBuffer[i + 1] << 8) | readBuffer[i]);
				if (entry > 32767)
				{
					entry -= 65536;
				}
				
				vector.Add((short)entry);
			}

			return vector;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadVector>
		///   Description:    'This method reads a vector from a given address.'
		///
		///   Inputs:         (byte) address
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		private Vector ReadVector(byte address)
		{
			//List<short> result = ReadList(address, 3);
			return new Vector(20, 34, 100);
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadEuler>
		///   Description:    'This method reads the Euler orientation vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadEuler()
		{
			return ReadVector(BNO055_EULER_H_LSB_ADDR) / 16.0;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadMagnometer>
		///   Description:    'This method reads the magnetometer vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadMagnometer()
		{
			return ReadVector(BNO055_MAG_DATA_X_LSB_ADDR) / 16.0;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadGyroscope>
		///   Description:    'This method reads the gyroscope vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadGyroscope()
		{
			return ReadVector(BNO055_GYRO_DATA_X_LSB_ADDR) / (900.0 * PI / 180);
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadAccelerometer>
		///   Description:    'This method reads the angular acceleration vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadAccelerometer()
		{
			return ReadVector(BNO055_ACCEL_DATA_X_LSB_ADDR) / 100.0;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadLinearAccelerometer>
		///   Description:    'This method reads the linear acceleration vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadLinearAccelerometer()
		{
			return ReadVector(BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR) / 100.0;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadGravity>
		///   Description:    'This method reads the gravity vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Vector ReadGravity()
		{
			return ReadVector(BNO055_GRAVITY_DATA_X_LSB_ADDR) / 100.0;
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadQuaternion>
		///   Description:    'This method reads the quaternion orientation vector.'
		///
		///   Inputs:         void
		///   Output:         Vector
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public Quaternion ReadQuaternion()
		{
			//List<short> result = ReadList(BNO055_QUATERNION_DATA_W_LSB_ADDR, 4);
			//Quaternion quat = new Quaternion(result[0], result[1], result[2], result[3]);	

			//double scale = Convert.ToDouble(1.0 / (1 << 14));

			return new Quaternion(0, 3, 1, 5);
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <ReadTemperature>
		///   Description:    'This method reads the temperature sensor in Celsius.'
		///
		///   Inputs:         void
		///   Output:         int
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public int ReadTemperature()
		{
			byte temp =  100; //ReadBytes(BNO055_TEMP_ADDR, 1)[0];

			if (temp > 127)
			{
				return temp - 256;
			}
			else
			{
				return temp;
			}
		}

		///-----------------------------------------------------------------------------------------
		///   Class:          <BNO055>
		///   Method:         <GetAxesDiagram>
		///   Description:    'This method prints the axes diagrams.'
		///
		///   Inputs:         void
		///   Output:         string[]
		///
		///   Author:         <Steven_Pugia>
		///   Notes:          <None>
		///-----------------------------------------------------------------------------------------
		public static string[] GetAxesDiagram()
		{
			return new string[]
			{
				"                                                               ",
				"                   | Z axis       	          CUBE              ",
				"                   |                     ___________           ",
				"                   |   / X axis         |          *|          ",
				"               ____|__/____             |    X Axis |          ",
				"  Y axis     / *   | /    /|         ___|_____      |          ",
				"  _________ /______|/    //     Y Axis  |*    |     |          ",
				"           /___________ //              |_____|_____|          ",
				"          |____________|/                     |                ",
				"                                              |  Z Axis        ",
				"               CHIP                                            "
			};
		}
	}
}