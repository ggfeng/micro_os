#include "touch_atmel1616.h"
#include "hardware/platform.h"
#define Touch_ADDW              0x60
#define Touch_ADDR              0x61

#define TouchID                         0x01
unsigned char I2C_MEMADD_SIZE=1;
unsigned char i2c_id_t=1;

unsigned char Touch_atmel616_init(void)
{
	if(Touch_Read(TouchID))
	{
		return 1;
	}
	return 0;
}
unsigned short int Touch_Read(unsigned char addr)
{
	unsigned char data[2]={0,0};
	unsigned char dat16=0;
	unsigned char i=0;
	unsigned char times=0;
	while((times<10)&(pal_i2c_read(i2c_id_t, Touch_ADDR, addr, I2C_MEMADD_SIZE,&data[0],2)!= 1))
	{
		times++;
	}
	dat16=data[0];
	dat16|=data[1]<<8;
	return dat16;
}
