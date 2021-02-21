#ifndef __DEVISL97900_H_
#define __DEVISL97900_H_
#if 0
#define	ISL97900_DEVICE_ADR					0x50

#define ISL97900_STATUSREG					0x01
#define CH0							0x01
#define CH1							0x02
#endif

typedef struct
{
	unsigned char Addr;
	unsigned char Data;
}ISL_TypeDef;


typedef struct
{
	unsigned char RedLEDLSB;
	unsigned char GreenLEDLSB;
	unsigned char BlueLEDLSB;
	unsigned char RGBLEDMSB;
}ISL_Current;


#define CURRENT_SET						16
#define CURRENT_REDLSB_REG					0x13
#define CURRENT_GREENLSB_REG					0x14
#define CURRENT_BLUELSB_REG					0x15
#define CURRENT_RGBMSB_REG					0x17



ISL_Current Isl97900_GainDAC[];

unsigned char DevISL97900_Init(unsigned char ch);
unsigned char isl97900_Write_Current(unsigned char ch, unsigned char vol);
#endif

