#include "hardware/devisl97900.h"
#include "string.h"
#include <hardware/platform.h>
#define ISL97900_DEVICE_ADR                                     0x50

#define ISL97900_STATUSREG                                      0x01
#define CH0                                                     0x01
#define CH1                                                     0x02



unsigned char black_i2c_id=1;
unsigned char black_I2C_SIZE=1;

ISL_TypeDef Isl97900_DefReg[] = {
										0x00, 0x00,
										0x01, 0x01,
										0x02, 0x04,
										0x03, 0x00,
										0x04, 0x82,
										0x05, 0x10,
										0x06, 0x00,
										0x07, 0x00,
										0x08, 0x00,
										0x09, 0x00,
										0x0A, 0x00,
										0x0B, 0x00,
										0x0C, 0x00,
										0x0D, 0x00,
										0x0E, 0x00,
										0x0F, 0x00,
										0x10, 0x00,
										0x11, 0x00,
										0x12, 0x00,
										0x13, 0x20,
										0x14, 0x20,
										0x15, 0x20,
										0x16, 0x00,
										0x17, 0x00,
										0x18, 0x66,
										0x19, 0x9a,
										0x1A, 0x9a,
										0x1B, 0x9a,
										0x1C, 0x9a,
										0x1D, 0x55,
										0x1E, 0x40,													};


// Iled = (GainDAC/1023) * (150mV/Rsense)
// Iledmax		= 1.5A
// GainDACmax	= 1023
// Rsense		= 100mΩ
//目前测试最大只支持320mA电流

ISL_Current Isl97900_GainDAC[] = {
		0x10, 0x10, 0x10, 0x00,	//23.46mA	//00
		0x18, 0x18, 0x18, 0x00,	//35.19mA	//01
		0x20, 0x20, 0x20, 0x00,	//46.92mA	//02
		0x28, 0x28, 0x28, 0x00,	//58.65mA	//03
		0x30, 0x30, 0x30, 0x00,	//70.38mA	//04
		0x38, 0x38, 0x38, 0x00,	//82.10mA	//05
		0x40, 0x40, 0x40, 0x00,	//93.84mA	//06
		0x48, 0x48, 0x48, 0x00,	//105.57mA	//07
		0x50, 0x50, 0x50, 0x00,	//117.30mA	//08
		0x58, 0x58, 0x58, 0x00,	//129.03mA	//09
		0x60, 0x60, 0x60, 0x00,	//140.76mA	//10
		0x68, 0x68, 0x68, 0x00,	//152.49mA	//11
		0x70, 0x70, 0x70, 0x00,	//164.22mA	//12
		0x78, 0x78, 0x78, 0x00,	//175.95mA	//13
		0x80, 0x80, 0x80, 0x00,	//187.68mA	//14
		0x88, 0x88, 0x88, 0x00,	//199.41mA	//15
		0x90, 0x90, 0x90, 0x00,	//211.14mA	//16
		0x98, 0x98, 0x98, 0x00,	//222.87mA	//17
		0xA0, 0xA0, 0xA0, 0x00,	//234.60mA	//18
		0xA8, 0xA8, 0xA8, 0x00,	//246.33mA	//19
		0xB0, 0xB0, 0xB0, 0x00,	//258.06mA	//20
		0xB8, 0xB8, 0xB8, 0x00,	//269.79mA	//21
		0xC0, 0xC0, 0xC0, 0x00,	//281.52mA	//22
		0xC8, 0xC8, 0xC8, 0x00,	//293.25mA	//23
		0xD0, 0xD0, 0xD0, 0x00,	//304.98mA	//24
		0xD8, 0xD8, 0xD8, 0x00,	//316.71mA	//25
		0xE0, 0xE0, 0xE0, 0x00,	//328.44mA	//26
								};

/////////////////////////////////////////////////////////////////////////////////////
// 输	入:
// 输	出:
// 创建日期:
// 功能描述: isl97900的任一地址读，最大地址0x29
// 创 建 者: 
// 修改日期:
// 修改备注:
/////////////////////////////////////////////////////////////////////////////////////
unsigned char isl97900_Read(unsigned char ch, unsigned char Addr, unsigned char  *pData, unsigned short int len)
{
	
//	unsigned char rtn=0;
	unsigned char isl_len=0;
	if( Addr > 0x29 ) return 0;
	
		isl_len=(unsigned char)len;
		pal_i2c_read(black_i2c_id, ISL97900_DEVICE_ADR+1, Addr, black_I2C_SIZE, pData,isl_len);
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
// 输	入:
// 输	出:
// 创建日期:	
// 功能描述: isl97900的任一地址写,最大地址0x1E
// 创 建 者: 
// 修改日期:
// 修改备注:
/////////////////////////////////////////////////////////////////////////////////////
unsigned char isl97900_Write(unsigned char ch, unsigned char Addr, unsigned char *pData, unsigned short int len)
{
	
	//unsigned char rtn = 0;

	if( Addr > 0x1E ) return 0;
	pal_i2c_write(black_i2c_id,ISL97900_DEVICE_ADR,Addr,black_I2C_SIZE, pData,len);
	return 1;
	
}

unsigned char isl97900_Write_Current(unsigned char ch,unsigned char vol)
{
	
 	//	unsigned char rtn = FALSE;
	unsigned char tmp[3] = {0};

	tmp[0] = (unsigned char)((Isl97900_GainDAC[vol].RedLEDLSB * 98) / 100);
	tmp[1] = (unsigned char)((Isl97900_GainDAC[vol].GreenLEDLSB * 1) / 1);
	tmp[2] = (unsigned char)((Isl97900_GainDAC[vol].BlueLEDLSB * 1) / 1);

	isl97900_Write(ch, CURRENT_REDLSB_REG, &tmp[0], 1);
	isl97900_Write(ch, CURRENT_GREENLSB_REG, &tmp[1], 1);
	isl97900_Write(ch, CURRENT_BLUELSB_REG, &tmp[2], 1);
	isl97900_Write(ch, CURRENT_RGBMSB_REG, &Isl97900_GainDAC[vol].RGBLEDMSB, 1);

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////
// 输	入:
// 输	出:
// 创建日期:	
// 功能描述: isl97900初始化
// 创 建 者:
// 修改日期:
// 修改备注:
/////////////////////////////////////////////////////////////////////////////////////
unsigned char DevISL97900_Init(unsigned char ch)
{
	
	unsigned char tmp = 0;
	unsigned char rtn = 0;
	unsigned char i;

	 isl97900_Read(ch,0x20, &tmp, 1 );
	if(rtn)
	{

		for(i = 0; i < (sizeof(Isl97900_DefReg) / sizeof(ISL_TypeDef)); i++ )
		{
			isl97900_Write(black_i2c_id, Isl97900_DefReg[i].Addr, &Isl97900_DefReg[i].Data, 1);
		}
	}else
	{
		return 0;
	}
	return 1;
}


