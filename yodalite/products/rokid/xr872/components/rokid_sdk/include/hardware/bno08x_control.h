#ifndef _BNO08X_CONTROL_H_
#define _BNO08X_CONTROL_H_
#include "string.h"
//#include "sh2.h"
//#include "shtp.h"
//#include "sh2_hal.h"
//#include "sh2_err.h"
//#include "sh2_SensorValue.h"
//#include "shtpI2CHal.h"
//#include "mxconstants.h"
#include "hardware/pal_i2c.h"
#include "hardware/platform.h"
#if 0
typedef struct 
{
	unsigned char 	fress;
	union 
	{
		uint64_t timestamp;
		unsigned char timestamp_buf[8];
	};
	 
	union 
	{
		sh2_Accelerometer_t accelerometer; 
		unsigned char 	accelerometer_buf[12];
	};
	union
	{
		sh2_Gyroscope_t gyroscope;
		unsigned char 	gyroscope_buf[12];
	};
	union
	{
		sh2_MagneticField_t magneticField;
		unsigned char  magneticField_buf[12];
	};
	union
	{
		sh2_RotationVectorWAcc_t rotationVector; 
		unsigned char	rotationVector_buf[20];
	};
	union
	{
		sh2_RotationVector_t gameRotationVector; 
		unsigned char gameRotationVector_buf[16];
	};
}IMU_Date;

extern IMU_Date	imu_data; 
#endif

/**/
void IMU_IT(uint16_t GPIO_Pin);
//void bno08x_control_Init(struct gpio_lapi * platform_gpio_lapi,struct i2c_lapi *platform_i2c_lapi);
void bno08x_control_Init(void);
void bno08x_control_run(void);
void bno08x_I2C1_Init(struct i2c_lapi *platform_i2c_lapi);
#endif

