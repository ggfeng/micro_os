#ifndef _BNO08X_PARAMETER_H_
#define _BNO08X_PARAMETER_H_
#include "string.h"
#include "sh2.h"
#include "shtp.h"
#include "sh2_hal.h"
#include "sh2_err.h"
#include "sh2_SensorValue.h"
#include "shtpI2CHal.h"
#include "mxconstants.h"

#define false	0
#define true	1
#define bno_hi2c2	2
typedef struct
{
        unsigned char   fress;
        union
        {
                uint64_t timestamp;
                unsigned char timestamp_buf[8];
        };

        union
        {
                sh2_Accelerometer_t accelerometer;
                unsigned char   accelerometer_buf[12];
        };
        union
        {
                sh2_Gyroscope_t gyroscope;
                unsigned char   gyroscope_buf[12];
        };
        union
        {
                sh2_MagneticField_t magneticField;
                unsigned char  magneticField_buf[12];
        };
        union
        {
                sh2_RotationVectorWAcc_t rotationVector;
                unsigned char   rotationVector_buf[20];
        };
        union
        {
                sh2_RotationVector_t gameRotationVector;
                unsigned char gameRotationVector_buf[16];
        };
}IMU_Date;

extern IMU_Date imu_data;


#endif

