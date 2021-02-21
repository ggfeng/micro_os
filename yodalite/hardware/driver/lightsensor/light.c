#include "hardware/light.h"
#include "hardware/platform.h"
#include "hardware/pal_i2c.h"
#define HAL_OK	1
#define SLight_AddrW                    0x52  //0X52
#define SLight_AddrR                    0x53    //0X53


#define ALS_CONTR                       0x80
#define ALS_MEAS_RATE                   0x85
#define PART_ID                         0x86
#define MANUFAC_ID                      0x87
#define ALS_DATA_CH1_0                  0x88
#define ALS_DATA_CH1_1                  0x89
#define ALS_DATA_CH0_0                  0x8a
#define ALS_DATA_CH0_1                  0x8b
#define ALS_STATUS                      0x8c
#define INTERRUPT                       0x8f
#define ALS_THRES_UP_0                  0x97
#define ALS_THRES_UP_1                  0x98
#define ALS_THRES_LOW_0                 0x99
#define ALS_THRES_LOW_1                 0x9a
#define INTERRUPT_PERSIST               0x9e    
#define LightID_Value                   0xa0


unsigned char light_I2C_MEMADD_SIZE=1;
unsigned char i2c_id_l=1;


static unsigned char ch=0xaa;
unsigned char Light_Init(void)
{
	if(Light_Read(PART_ID)==LightID_Value)
	{
	
	}else
	{
		return 0;
	}


		//Init	Light
		Light_Write(ALS_MEAS_RATE,0x03);//500ms
		Light_Write(INTERRUPT,0x02);//
		Light_Write(ALS_CONTR,0x01);

		Light_Read(ALS_STATUS);
		ch=0;
	return 1;
}
unsigned short int Light_DataRead(void)
{
	unsigned char date[2];
	unsigned short int dat16=0;
	if(ch==0)
	{
		 date[0]=Light_Read(ALS_DATA_CH1_0);
		 date[1]=Light_Read(ALS_DATA_CH1_1);
		ch=1;
	}else if(ch==1)
	{
		 date[0]=Light_Read(ALS_DATA_CH0_0);
		 date[1]=Light_Read(ALS_DATA_CH0_1);	
		ch=0;
	}else
	{
		Light_Read(ALS_CONTR);
		Light_Read(ALS_MEAS_RATE);	
	}
	dat16=date[1]<<8;
	dat16|=date[0];
	return dat16;
}
void Light_Write(unsigned char addr,unsigned char com)
{
	unsigned char i=0;
	while((i<10)&(pal_i2c_write(i2c_id_l,SLight_AddrW, addr,light_I2C_MEMADD_SIZE,&com,2)!= HAL_OK))
	{
		i++;
	}
	
}

unsigned char Light_Read(unsigned char addr)
{
	unsigned char data=0;
	unsigned char i=0;
	while((i<10)&(pal_i2c_read(i2c_id_l, SLight_AddrR, addr, light_I2C_MEMADD_SIZE,&data,1)!= HAL_OK))
	{
		i++;
	}
	
	return data;
}
