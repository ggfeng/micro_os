#include "hardware/usb_hub.h"
#include "hardware/platform.h"
#include "hardware/pal_i2c.h"
#define         USBHUB_ADD                      0x10
#define         USBHUB_ADDR                     ((USBHUB_ADD<<0)+1)
#define         USBHUB_ADDW                     ((USBHUB_ADD<<0)+0)


#define         HUBVIDL                                 0x00
#define HUBVIDM                         0x01
#define         HUBPIDL                                 0x02
#define         HUBPIDM                         0x03
#define         HUBDIDL                                 0x04
#define HUBDIDM                         0x05
#define   HUBCFG1                                       0x06
#define   HUBCFG2                                       0x07
#define   HUBCFG3                                       0x08
#define NRD                                     0x09
#define PDS                                     0x0a
#define PDB                                     0x0b
#define MAXPS                                   0x0c
#define MAXPB                                   0X0d
#define HCMCS                                   0x0e
#define HCMCB                                   0x0f
#define PWRT                                    0x10
#define LANGIDH                                 0x11
#define LANGIDL                                 0x12
#define MFRSL                                   0x13
#define PRDSL                                   0x14
#define SERSL                                   0x15

#define  I2C_SIZE_8BIT                          8


unsigned i2c_id=1;
unsigned char I2C_SIZE=1;

unsigned char usbhub_init(void)
{
		unsigned char  data[3]={0,0,0};
		unsigned char  cg1=0x8b;
		unsigned char  cg2=0x08;
		unsigned char  cg3=0x09;
		unsigned char  max_c=0xf0;

		pal_i2c_read( i2c_id,  USBHUB_ADDR, HUBCFG1,  I2C_SIZE,&data[0],1);
		pal_i2c_read( i2c_id,  USBHUB_ADDR, HUBCFG2,  I2C_SIZE,&data[1],1);
		pal_i2c_read(i2c_id,  USBHUB_ADDR, HUBCFG3,  I2C_SIZE,&data[2],1);
		if(data[1])
		{
				
		}else
		{
			return 0;		//Periphery_State.Hub=NO_EXIST;
		}

		pal_i2c_write( i2c_id,USBHUB_ADDW, HCMCB,  I2C_SIZE,&max_c,1);
		pal_i2c_write( i2c_id,USBHUB_ADDW, HCMCS,  I2C_SIZE,&max_c,1);
		pal_i2c_write( i2c_id,USBHUB_ADDW, MAXPS,  I2C_SIZE,&max_c,1);
		pal_i2c_write(i2c_id, USBHUB_ADDW, MAXPB,  I2C_SIZE,&max_c,1);		
		pal_i2c_write(i2c_id, USBHUB_ADDW, HUBCFG1,I2C_SIZE,&cg1,1);
		pal_i2c_write(i2c_id, USBHUB_ADDW, HUBCFG2,I2C_SIZE,&cg2,1);
		pal_i2c_write(i2c_id, USBHUB_ADDW, HUBCFG3,I2C_SIZE,&cg3,1);
	//	pal_i2c_write(i2c_id,  USBHUB_ADDR, HUBCFG1,  I2C_SIZE_8BIT,&data[0],1);
		return 1;
}
