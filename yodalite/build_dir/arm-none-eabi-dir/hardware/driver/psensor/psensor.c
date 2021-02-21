#include "hardware/psensor.h"
#include "hardware/platform.h"
#include "psensor_parameter.h"
#include "hardware/pal_i2c.h"

unsigned short int psensor_read(unsigned char addr);
void psensor_write(unsigned char addr,unsigned char *data);
#define         Psensor_ADDR    0xc1
#define         Psensor_ADDW    0xc0
#define         PConfig_Num             8


#define         ALS_CONF1                       0x00      //0x80 0x0c
#define         ALS_THDH                        0x01                    //0x01 0x00
#define         ALS_THDL                        0x02                    //0x00 0x10
#define         PS_CONF1                        0x03                    //0x0a 0x09
#define         PS_CONF3                        0x04                    //0x04 0x80
#define         PS_THDL                         0x05                    //0x00 0x20
#define         PS_THDH                         0x06                    //0x01 0x00
#define         PS_CANC                         0x07                    //0x00 0x00
#define         PS_AC                           0x08                    //0x0d 0x00
#define         ALS_DATA                        0xf1                    //
#define         IR_Data                         0xf3
#define         PS_Data                         0xf4
#define         INT_Flag                        0xf5
#define         ID                              0xf6
#define         PS_AC_Data                      0xf7


#define		ALS_CONF1			0x00      //0x80 0x0c
#define		ALS_THDH			0x01			//0x01 0x00
#define		ALS_THDL			0x02			//0x00 0x10
#define		PS_CONF1			0x03			//0x0a 0x09
#define		PS_CONF3			0x04			//0x04 0x80
#define		PS_THDL				0x05			//0x00 0x20
#define     PS_THDH				0x06			//0x01 0x00
#define		PS_CANC				0x07			//0x00 0x00
#define		PS_AC				0x08			//0x0d 0x00
unsigned char psensor_initData[]={
														0x82,0x03,				//ALS CONFIG
														0x10,0x10,				//ALS H INTERRUPT
														0x10,0x01,				//ALS L	INTERRUPT
														0x0a,0x09,				//PSENSOR CONFIG1
														0x04,0x80,				//PSENSOR CONFIG2
														0x16,0x00,				//PSENSOR	H
														0x08,0x00,				//PSENSOR L
														0x00,0x00,				//
														0x0d,0x00					//	
														};
unsigned char psensor_init(void)
{
	unsigned char i=0;
	unsigned short int id=0;
	unsigned char pdata[2];
	id=psensor_read(0xF6);
	if(id)
	{
			for(i=0;i<=PConfig_Num;i++)
			{
				psensor_write(i,&psensor_initData[2*i]);
			}	/**/
			#if DEBUG
				for(i=0;i<=PConfig_Num;i++)
				{
					psensor_read(i);
				}	
			#endif
	}else
	{
		return 0;
	}
	return 1;
}
unsigned short psensor_readstatus(void)
{
	return psensor_read(INT_Flag);
}
unsigned char psensor_readdata(void)
{
	unsigned short int light=0;
	
	light=psensor_read(ALS_DATA);
	if(light<0x100)
		return 0;
	else
		return 1;
}

unsigned short int psensor_read(unsigned char addr)
{
	unsigned char i=0;
	unsigned char data[2]={0,0};
	unsigned short int data16=0;
	while((i<10)&(!pal_i2c_read(p_i2c_id, Psensor_ADDR, addr, psensor_I2C_MEMADD_SIZE,&data[0],2)))
	{
		i++;
	}
	data16 =data[0];
	data16|=data[1]<<8;
	return data16;
}
void psensor_write(unsigned char addr,unsigned char *data)
{
	unsigned char i;
	unsigned short int data16=0;
	while((i<10)&(!pal_i2c_write(p_i2c_id,Psensor_ADDW, addr, psensor_I2C_MEMADD_SIZE,data,2)))
	{
		i++;
	}

}

