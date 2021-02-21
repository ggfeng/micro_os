#include "stm32flash.h"
#include "main.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_flash.h"
#include "data.h"
void EraseByte(unsigned long int addr)
{
	uint32_t beginTick =0,endTick =0;
	uint32_t curSysTick=0,endSysTick =0;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef f;
	unsigned short int i;
	
	__disable_irq();
	HAL_FLASH_Unlock();
	//f.Banks=1;
	if(addr==MCUA)
	{
		f.Sector=FLASH_SECTOR_5;
	}else if(addr==MCUB)
	{
		f.Sector=FLASH_SECTOR_6;
	}else if(addr==MCUC)
	{
		f.Sector=FLASH_SECTOR_7;
	}
	f.NbSectors=1;
	f.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	f.TypeErase = FLASH_TYPEERASE_SECTORS;	
	
//	while(HAL_FLASHEx_Erase(&f, &PageError)!=HAL_OK)
//	{
		//delay(50000);
//	}
//	FLASH_WaitForLastOperation(100000);
//	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
//	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY|FLASH_FLAG_PGAERR|FLASH_FLAG_EOP);
	
		curSysTick =SysTick->VAL;
		FLASH_WaitForLastOperation(100000);
		//while(FLASH_SR);
		HAL_FLASHEx_Erase(&f, &PageError);
//	while(HAL_FLASHEx_Erase(&f, &PageError)!= HAL_OK) //??sector8HAL_FLASHEx_Erase_IT(&f)
//	{
//    Error_Handler();
//		//FLASH_WaitForLastOperation(100000);//100000
//		FLASH_WaitForLastOperation(10000000);
//	//	delay(50000);
//	}
//	//delay(50000);
		endSysTick =SysTick->VAL;
		HAL_FLASH_Lock();
	__enable_irq();

}

void readByte(unsigned long int addr,unsigned char *data,unsigned short int size)
{
	unsigned short int i=0;
	for(i=0;i<size;i++)
	{
		*(data+i)=*(unsigned char *)(addr+i);
	}
}
void writeflash(unsigned long int addr,unsigned char *data,unsigned short int size)
{
	unsigned short int i=0;
	__disable_irq();
	HAL_FLASH_Unlock();
	//FLASH_CleaRFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP);//FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	for(i=0;i<size;i++)
	{
			assert_param(IS_FLASH_ADDRESS(addr+i));
			CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);	
			FLASH->CR |= FLASH_PSIZE_BYTE;
			FLASH->CR |= FLASH_CR_PG;		
			  *(__IO uint8_t*)(addr+i) = *(data+i);
	}
	HAL_FLASH_Lock();
	__enable_irq();
}
void Down_FileData(void)
{
	unsigned char i=0;
	readByte(File_Flash,&data_file.data[0],FILESIZE+3);
	data_file.vision[0]=data_file.data[0];
	data_file.vision[1]=data_file.data[1];
	data_file.vision[2]=data_file.data[2];
	data_file.vision[3]=data_file.data[3];
	data_file.vision[4]=data_file.data[4];
	data_file.SN_size=data_file.data[5];
	for(i=0;i<64;i++)
	{
		data_file.SN[i]=data_file.data[6+i];
	}
	for(i=0;i<32;i++)
	{
		data_file.pcbaSn[i]=data_file.data[70+i];
	}
	
	for(i=0;i<32;i++)
	{
		data_file.TpyeId[i]=data_file.data[102+i];
	}	
	
	data_file.Lim_ALS[0]=data_file.data[134];
	data_file.Lim_ALS[1]=data_file.data[135];
	
	data_file.Lim_PS[0]=data_file.data[136];
	data_file.Lim_PS[1]=data_file.data[137];
	
	data_file.addr[0]=data_file.data[138];
	data_file.addr[1]=data_file.data[139];
	data_file.addr[2]=data_file.data[140];
	data_file.addr[3]=data_file.data[141];
}

void UP_FileData(void)
{
	unsigned char i;
	unsigned char k;
	uint32_t beginTick =0,endTick =0;
	uint32_t curSysTick=0,endSysTick =0;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef f;
	__disable_irq();
	HAL_FLASH_Unlock();
//	f.Banks=1;
	f.Sector=FLASH_SECTOR_1;
	f.NbSectors=1;
	f.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	f.TypeErase = FLASH_TYPEERASE_SECTORS;//FLASH_TYPEERASE_SECTORS
//	FLASH_CleaRFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	curSysTick =SysTick->VAL;
	
	while(HAL_FLASHEx_Erase(&f, &PageError)!= HAL_OK) //??sector8HAL_FLASHEx_Erase_IT(&f)
	{
    Error_Handler();
		FLASH_WaitForLastOperation(10000);
	}
	//;
	endSysTick =SysTick->VAL;
	HAL_FLASH_Lock();
	__enable_irq();
	
	data_file.data[0]=data_file.vision[0];
	data_file.data[1]=data_file.vision[1];
	data_file.data[2]=data_file.vision[2];
	data_file.data[3]=data_file.vision[3];
	data_file.data[4]=data_file.vision[4];
	data_file.data[5]=data_file.SN_size;
	for(i=0;i<64;i++)
	{
		data_file.data[6+i]=data_file.SN[i];
	}
	
	for(i=0;i<32;i++)
	{
		data_file.data[70+i]=data_file.pcbaSn[i];
	}
	
	for(i=0;i<32;i++)
	{
		data_file.data[102+i]=data_file.TpyeId[i];
	}	
	
	data_file.data[134]=data_file.Lim_ALS[0];
	data_file.data[135]=data_file.Lim_ALS[1];
	
	data_file.data[136]=data_file.Lim_PS[0];
	data_file.data[137]=data_file.Lim_PS[1];
	
	data_file.data[138]=data_file.addr[0];
	data_file.data[139]=data_file.addr[1];
	data_file.data[140]=data_file.addr[2];
	data_file.data[141]=data_file.addr[3];
	data_file.data[142]=0xff;
	data_file.data[143]=0xff;
	data_file.data[144]=0xff;
//	data_file.data[145]=0xff;
//	data_file.data[146]=0xff;
//	data_file.data[147]=0xff;
	writeflash(File_Flash,&data_file.data[0],FILESIZE+3);
}




//void HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);


