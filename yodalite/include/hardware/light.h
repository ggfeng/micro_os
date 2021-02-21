#ifndef __LIGHT_H
#define __LIGHT_H
#if 0
#define SLight_AddrW			0x52  //0X52
#define	SLight_AddrR			0x53	//0X53


#define	ALS_CONTR			0x80
#define	ALS_MEAS_RATE			0x85
#define PART_ID				0x86
#define	MANUFAC_ID			0x87
#define	ALS_DATA_CH1_0			0x88
#define ALS_DATA_CH1_1			0x89
#define	ALS_DATA_CH0_0			0x8a
#define	ALS_DATA_CH0_1			0x8b
#define	ALS_STATUS			0x8c
#define	INTERRUPT			0x8f
#define	ALS_THRES_UP_0			0x97
#define ALS_THRES_UP_1			0x98
#define ALS_THRES_LOW_0			0x99
#define ALS_THRES_LOW_1			0x9a
#define INTERRUPT_PERSIST		0x9e	
#define	LightID_Value			0xa0
#endif

unsigned char Light_Init(void);
unsigned short int Light_DataRead(void);
unsigned char Light_Read(unsigned char addr);
void Light_Write(unsigned char addr,unsigned char com);
#endif
