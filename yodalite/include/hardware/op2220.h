#ifndef _OP2220_H_
#define _OP2220_H_
#include "hardware/platform.h"


//OP02220 device define
#define OP02220_V1S0_GPIO_PORT                          1
#define OP02220_V1S0_PIN                                6
#define OP02220_RESET0_GPIO_PORT                        1
#define OP02220_RESET0_PIN                              7



#define BOOLEAN_TypeDef 	unsigned char
#define OP2220_DEVICE_ADR	0xca
#define OP2220AddrSize		2
#define OP2220_DEV_CHx		1

#define COUNT_SIZE		32

#define	P_OP02220_V1S0_Set_H					pal_rawgpio_direction_output(OP02220_V1S0_GPIO_PORT, OP02220_V1S0_PIN, 1)
#define	P_OP02220_V1S0_Set_L					pal_rawgpio_direction_output(OP02220_V1S0_GPIO_PORT, OP02220_V1S0_PIN, 0)
#define	P_OP02220_Reset0_H					pal_rawgpio_direction_output(OP02220_RESET0_GPIO_PORT, OP02220_RESET0_PIN, 1)
#define	P_OP02220_Reset0_L					pal_rawgpio_direction_output(OP02220_RESET0_GPIO_PORT, OP02220_RESET0_PIN, 0)


// External Addresses
#define	REG_ADDR_START					0x0000
#define	MIPI_REGISTER_START				0x0100
#define	GAMMA_TABLE_START				0x0800
#define	PWM_TABLE_START					0x0900
#define	LSB_TABLE_START					0x0A00
#define	MEM_ADDR_START					0x8000

// Count sizes
#define	REGISTER_COUNT					256
#define	MIPI_REG_COUNT					256
#define	GAMMA_COUNT					256
#define	PWM_COUNT					64
#define	LSB_TABLE_COUNT					256
#define	SEQUENCE_COUNT					1024

// Color Selection
#define	RED_SELECT					0x0000
#define	GREEN_SELECT					0x1000
#define	BLUE_SELECT					0x2000

// OP02220 registers
#define	MEM_MODE_REG					0x01F
#define	LED_Ctrl					0x08c
#define	VSET						0x08d
#define	Table_ctrl					0x090
#define	DRAM_Rfsh					0x091
#define	Phy_Ctrl0					0x120
#define	SPECIAL_REG_SELECT				0x7FFE
#define AT24ChipQuantity 0


#define	NV_REGISTERS					0x0000
#define	NV_MIPI_REG					0x0100
#define	NV_GAMMA_RED					0x0200
#define	NV_GAMMA_GREEN					0x0300
#define	NV_GAMMA_BLUE					0x0400
#define	NV_PWM_RED					0x0500
#define	NV_PWM_GREEN					0x0600
#define	NV_PWM_BLUE					0x0700
#define	NV_LSB_TABLE					0x0800
#define	NV_SEQUENCE					0x0900
unsigned char DevOp02220_Init(uint8_t ch);

unsigned char SCCB_Burst_Read_Char(uint8_t ch, uint8_t dev, uint16_t Addr, uint8_t *pData, uint16_t len);


#endif
