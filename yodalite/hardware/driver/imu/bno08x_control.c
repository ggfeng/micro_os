#include "hardware/bno08x_control.h"
#include "shtpI2CHal.h"
#include "bno08x_parameter.h"
#include "hardware/platform.h"
#include "hardware/pal_i2c.h"
#include "string.h"

//I2C_HandleTypeDef hi2c2;

void delay_ms(unsigned short int t)
{
	unsigned char i;
	for(;t>0;t--)
	 for(i=121;i>0;i--)
	{
		;
	}
}

IMU_Date  imu_data; 
sh2_ProductIds_t prodIds;
unsigned char resetPerformed = false;
unsigned char startedReports = false;
unsigned char sensorReceived = false;
sh2_SensorEvent_t sensorEvent;


void bno08x_GPIO_Init(struct gpio_lapi * platform_gpio_lapi);
void startReports(void);
void printEvent(const sh2_SensorEvent_t * event);
/* Private function prototypes -----------------------------------------------*/

void eventHandler(void * cookie, sh2_AsyncEvent_t *pEvent)
{
    if (pEvent->eventId == SH2_RESET) {
        resetPerformed = true;
    }
}

void sensorHandler(void * cookie, sh2_SensorEvent_t *pEvent)
{
    sensorEvent = *pEvent;
	//	xSemaphoreGive(wakeSensorTask1);
    sensorReceived = true;
}


void startReports(void)
{
    static sh2_SensorConfig_t config;
    int status;
    int sensorId;
        
  //  printf("Starting Sensor Reports.\n");

    config.changeSensitivityEnabled = true;
    config.wakeupEnabled = true;
    config.changeSensitivityRelative = true;
    config.alwaysOnEnabled = false;
    config.changeSensitivity = 0;
    config.reportInterval_us = 1000;  // microseconds (100Hz)
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    sensorId = SH2_ROTATION_VECTOR;
    status = sh2_setSensorConfig(sensorId, &config);
    if (status != 0) {
    //    printf("Error while enabling sensor %d\n", sensorId);
    }
//		delay_ms(2000);
    config.changeSensitivityEnabled = true;
    config.wakeupEnabled = true;
    config.changeSensitivityRelative = true;
    config.alwaysOnEnabled = false;
    config.changeSensitivity = 0;
    config.reportInterval_us = 1000;  // microseconds (100Hz)
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    sensorId = SH2_ACCELEROMETER;
    status = sh2_setSensorConfig(sensorId, &config);
    if (status != 0) {
      //  printf("Error while enabling sensor %d\n", sensorId);
    }		

//delay_ms(2000);
    config.changeSensitivityEnabled = true;
    config.wakeupEnabled = true;
    config.changeSensitivityRelative = true;
    config.alwaysOnEnabled = true;//false
    config.changeSensitivity = 0;
    config.reportInterval_us = 1000;  // microseconds (100Hz)
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    sensorId = SH2_GYROSCOPE_CALIBRATED;
    status = sh2_setSensorConfig(sensorId, &config);
    if (status != 0) {
      //  printf("Error while enabling sensor %d\n", sensorId);
    }				
		
    delay_ms(2000);
    config.changeSensitivityEnabled = true;
    config.wakeupEnabled = true;
    config.changeSensitivityRelative = true;
    config.alwaysOnEnabled = true;//false
    config.changeSensitivity = 0;
    config.reportInterval_us = 1000;  // microseconds (100Hz)
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    sensorId = SH2_MAGNETIC_FIELD_CALIBRATED;
    status = sh2_setSensorConfig(sensorId, &config);
    if (status != 0) {
      //  printf("Error while enabling sensor %d\n", sensorId);
    }			
				delay_ms(2000);
    config.changeSensitivityEnabled = true;
    config.wakeupEnabled = true;
    config.changeSensitivityRelative = true;
    config.alwaysOnEnabled = true;//false
    config.changeSensitivity = 0;
    config.reportInterval_us = 1000;  // microseconds (100Hz)
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    sensorId = SH2_GAME_ROTATION_VECTOR;
    status = sh2_setSensorConfig(sensorId, &config);
    if (status != 0) {
      //  printf("Error while enabling sensor %d\n", sensorId);
    }		
}


void printEvent(const sh2_SensorEvent_t * event)
{
    int rc;
    sh2_SensorValue_t value;
    float scaleRadToDeg = 180.0 / 3.14159265358;
    float r, i, j, k, acc_deg, x, y, z;
    float t;

    rc = sh2_decodeSensorEvent(&value, event);
    if (rc != SH2_OK) {
      //  printf("Error decoding sensor event: %d\n", rc);
        return;
    }
    t = value.timestamp / 1000000.0;  // time in seconds.
    imu_data.timestamp=value.timestamp ;
	    switch (value.sensorId) {
				case SH2_MAGNETIC_FIELD_CALIBRATED:
					imu_data.magneticField.x=value.un.magneticField.x;
					imu_data.magneticField.y=value.un.magneticField.y;
					imu_data.magneticField.z=value.un.magneticField.z;
				break;
		case SH2_GYROSCOPE_CALIBRATED:
					imu_data.gyroscope.x=value.un.gyroscope.x;
					imu_data.gyroscope.y=value.un.gyroscope.y;
					imu_data.gyroscope.z=value.un.gyroscope.z;
				
				break;
        case SH2_RAW_ACCELEROMETER:

            break;

        case SH2_ACCELEROMETER:
				imu_data.accelerometer.x=value.un.accelerometer.x;
				imu_data.accelerometer.y=value.un.accelerometer.y;
				imu_data.accelerometer.z=value.un.accelerometer.z;
						
            break;
        case SH2_ROTATION_VECTOR:
				imu_data.rotationVector.real=value.un.rotationVector.real;
				imu_data.rotationVector.i=value.un.rotationVector.i;
				imu_data.rotationVector.j=value.un.rotationVector.j;
				imu_data.rotationVector.k=value.un.rotationVector.k;  
            break;
        case SH2_GYRO_INTEGRATED_RV:

            break;
        case SH2_LINEAR_ACCELERATION:

            break;
        case SH2_GAME_ROTATION_VECTOR:
				imu_data.gameRotationVector.real=value.un.gameRotationVector.real;
				imu_data.gameRotationVector.i=value.un.gameRotationVector.i;
				imu_data.gameRotationVector.j=value.un.gameRotationVector.j;
				imu_data.gameRotationVector.k=value.un.gameRotationVector.k;
            break;
        default:
            break;
    }
}
void bno08x_control_Init(void)//struct gpio_lapi * platform_gpio_lapi,struct i2c_lapi *platform_i2c_lapi)
{
		//	bno08x_GPIO_Init(platform_gpio_lapi);
		//	bno08x_I2C1_Init(platform_i2c_lapi);
			sh2_hal_init(bno_hi2c2); 
			shtp_init();
			// init SH2 layer
			sh2_initialize(eventHandler, NULL);
			// Register event listener
			sh2_setSensorCallback(sensorHandler, NULL);
}
void bno08x_control_run(void)
{
	 halTask();
	if (sensorReceived) {
          sensorReceived = false;
          printEvent(&sensorEvent);
					}
	if (resetPerformed) {
	  resetPerformed = false;
          static sh2_ProductIds_t pProdIds;
          sh2_getProdIds(&pProdIds);
//	  Periphery_State.Imu=EXIST;
          startReports();
      }
}
void bno08x_GPIO_Init(struct gpio_lapi * platform_gpio_lapi)
{
	pal_gpio_init(platform_gpio_lapi);
}
void bno08x_I2C1_Init(struct i2c_lapi *platform_i2c_lapi)
{
	 pal_i2c_init(platform_i2c_lapi);
}
