#ifndef	DATA_H
#define	DATA_H
#include "string.h"
//#include "sh2.h"
//#include "shtp.h"
//#include "sh2_hal.h"
//#include "sh2_err.h"
//#include "sh2_SensorValue.h"
#include "stm32f4xx_hal.h"
//#include "mxconstants.h"
#define		Imu_Bno08x					  1
#define 	Touchu_AW9163					1
#define		Lsensor_LTR_303ALS		1
#define 	Psensor_VCNL3683			1
#define 	HUB_USB3803						1

#define 	DevlSL97900						1
#define 	DEBUG									0

#define 	MAXCOM_Parameter 			10

#define 	OPEN									0X55
#define		CLOSE									0xaa

#define 	EXIST									0xaa
#define 	NO_EXIST							0x00
#define 	Fail_Times						10
#define CAMERA_PORT							GPIOA
#define CAMERA_PIN							GPIO_PIN_15
#define	CAMERA_H												HAL_GPIO_WritePin(CAMERA_PORT, CAMERA_PIN		, GPIO_PIN_SET)
#define	CAMERA_L												HAL_GPIO_WritePin(CAMERA_PORT, CAMERA_PIN	, GPIO_PIN_RESET)


#define FILESIZE							84+32+32
#define Sleep									0x10
#define Down									0x20
#define	Week									0x00

#define MCUA									0x08020000
#define MCUB									0x08040000
#define MCUC									0x08060000

typedef struct 
{
	unsigned char Psensor;
	unsigned char Lsensor;
	unsigned char Touch;
	unsigned char BlackLight;
	unsigned char OpticalWaveguide;
	unsigned char Imu;
	unsigned char Hub;
	
	unsigned char previous_touch;
	unsigned char status;
	
}Sensor_Stata;

extern Sensor_Stata Periphery_State;
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
//		sh2_Accelerometer_t accelerometer; 
		unsigned char 	accelerometer_buf[12];
	};
	union
	{
//		sh2_Gyroscope_t gyroscope;
		unsigned char 	gyroscope_buf[12];
	};
	union
	{
//		sh2_MagneticField_t magneticField;
		unsigned char  magneticField_buf[12];
	};
	union
	{
//		sh2_RotationVectorWAcc_t rotationVector; 
		unsigned char	rotationVector_buf[20];
	};
	union
	{
//		sh2_RotationVector_t gameRotationVector; 
		unsigned char gameRotationVector_buf[16];
	};
	
	unsigned char touch_data[2];
	unsigned char light_data[3];
	unsigned char psensor;
	unsigned char key;
	unsigned char cam;
	unsigned char sleep;
	unsigned char error;
	unsigned char status;
}HID_SentDate;

typedef struct 
{
	unsigned char vision[5];
	char soft[5];
	unsigned char SN_size;
	char SN[64];
	char pcbaSn[32];
	char TpyeId[32];
	char Lim_ALS[2];
	char Lim_PS[2];
	union
	{
		unsigned long int run_list;
		unsigned char addr[4];
	};
	
	unsigned long int down_list;
	unsigned char data[FILESIZE+3];
}Date_File;



struct cmd_tbl_s {
	char 		*name;
	int			maxargs;	/* maximum number of arguments	*/
	unsigned char size;
	int			(*cmd)(int, char *[]);
};

#define CMD_NUM			25

typedef struct cmd_tbl_s	cmd_tbl_t;

typedef struct  
{
		cmd_tbl_t cmd[CMD_NUM];
		unsigned char com_num;

	
}cmd_tbl;


extern cmd_tbl	At_cmd;
extern Date_File	data_file;
extern HID_SentDate	hid_data; 
extern I2C_HandleTypeDef hi2c1;
extern void   find_cmd (char *cmd);
extern void   delay(unsigned short int t);
extern void   cmd_init(void);
#endif

