#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <hardware/platform.h>
#include <hardware/pal_i2c.h>
//#include <main.h>
#include "stm32f4xx_hal.h"
#include <vendorapi.h>
#define fail_times	10
//I2C_HandleTypeDef hi2c1;
//I2C_HandleTypeDef hi2c2;



int stm32_i2c_bus_init(struct i2c_resource *pi2c_res)
{
	unsigned id=(unsigned int)(pi2c_res->i2c_id);
	unsigned i2c_clk=(unsigned int)(pi2c_res->i2c_clk);
	if(id==1)
	{
 		hi2c1.Instance = I2C1;
 		hi2c1.Init.ClockSpeed = i2c_clk;
 		hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  		hi2c1.Init.OwnAddress1 = 0;
  		hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  		hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  		hi2c1.Init.OwnAddress2 = 0;
 		hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  		hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  		if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  		{
    			Error_Handler();
  		}
	}else if(id==2)
	{
  		hi2c2.Instance = I2C2;
  		hi2c2.Init.ClockSpeed = i2c_clk;
  		hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  		hi2c2.Init.OwnAddress1 = 0;
  		hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  		hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  		hi2c2.Init.OwnAddress2 = 0;
  		hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  		hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  		if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  		{
   		 Error_Handler();
  		}	
	
	}
   

}
int stm32_i2c_read(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode, unsigned char *pdata,unsigned char len)
{
	unsigned char i=0;
		i=0;
		if(i2c_id==1)
		{
			while((i<fail_times)&(HAL_I2C_Mem_Read(&hi2c1,client, addr,mode,&pdata,len,0xff)!= HAL_OK))
			{
				i++;
			}

		}else if(i2c_id==2)
		{
			while((i<fail_times)&(HAL_I2C_Mem_Read(&hi2c2,client,addr,mode,pdata,len,0xff)!=HAL_OK))
			{
				i++;
			}
		
		}
	if(i<fail_times)
		return 1;
	else 
		return 0;
}

int stm32_i2c_write(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode, unsigned char *pdata,unsigned char len)
{
		unsigned char i=0;
		i=0;
		if(i2c_id==1)
		{
			while((i<fail_times)&(HAL_I2C_Mem_Write(&hi2c1,client,addr,mode,pdata,len,0xff)!= HAL_OK))
			{
				i++;
		
			}
	


		}else if(i2c_id==2)
		{
			while((i<fail_times)&(HAL_I2C_Mem_Write(&hi2c2,client,addr,mode,pdata,len,0xff)!= HAL_OK))
			{
				i++;		
			}
			
		}
	if(i<fail_times)
		return 1;
	else 
		return 0;
}
int stm32_i2c_block_read(unsigned i2c_id,unsigned char client,unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len)
{
		if(i2c_id==1)
		{
			HAL_I2C_Master_Receive(&hi2c1, addr, pdata, len, 100);
		}else if(i2c_id==2)
		{
			HAL_I2C_Master_Receive(&hi2c2, addr, pdata, len, 100);
		}
}
int stm32_i2c_block_write(unsigned i2c_id,unsigned char client,unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len)
{
		if(i2c_id==1)
		{
			return HAL_I2C_Master_Receive(&hi2c1,addr, pdata, len, 100);
		}else if(i2c_id==2)
		{
			return HAL_I2C_Master_Receive(&hi2c2,addr, pdata, len, 100);
		}
	
}

struct i2c_lapi pal_i2c_lapi={
.yodalite_i2c_bus_init = stm32_i2c_bus_init,
.yodalite_i2c_read = stm32_i2c_read,
.yodalite_i2c_write = stm32_i2c_write,
.yodalite_i2c_block_read = stm32_i2c_block_read,
.yodalite_i2c_block_write = stm32_i2c_block_write,
};
