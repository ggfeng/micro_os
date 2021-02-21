#ifndef _PSENSOR_H_
#define	_PSENSOR_H_
#if 0
#define 	Psensor_ADDR	0xc1
#define		Psensor_ADDW	0xc0
#define		PConfig_Num		8


#define		ALS_CONF1			0x00      //0x80 0x0c
#define		ALS_THDH			0x01			//0x01 0x00
#define		ALS_THDL			0x02			//0x00 0x10
#define		PS_CONF1			0x03			//0x0a 0x09
#define		PS_CONF3			0x04			//0x04 0x80
#define		PS_THDL				0x05			//0x00 0x20
#define		PS_THDH				0x06			//0x01 0x00
#define		PS_CANC				0x07			//0x00 0x00
#define		PS_AC				0x08			//0x0d 0x00
#define 	ALS_DATA			0xf1			//
#define		IR_Data				0xf3
#define		PS_Data				0xf4
#define		INT_Flag			0xf5
#define		ID				0xf6
#define		PS_AC_Data			0xf7
#endif




unsigned char psensor_init(void);
unsigned short int psensor_readstatus(void);
unsigned char psensor_readdata(void);
#endif

