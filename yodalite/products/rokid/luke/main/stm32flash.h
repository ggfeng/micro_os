#ifndef _STM32FLASH_H_
#define	_STM32FLASH_H_


#define Boot_Flash			0x08000000
#define Boot_Section		0
#define Boot_Size				16

#define File_Flash			0x08004000U
#define File_Section		1
#define File_Size				16

#define RunA_Flash			0x08020000U
#define RunA_Section		5
#define RunA_Size				128

#define RunB_Flash			0x08040000U
#define RunB_Section		6
#define RunB_Size				128

#define RunC_Flash			0x08060000U
#define RunC_Section		7
#define RunC_Size				128

void writeflash(unsigned long int addr,unsigned char *data,unsigned short int size);
void readByte(unsigned long int addr,unsigned char *data,unsigned short int size);
void EraseByte(unsigned long int addr);
void Down_FileData(void);
void UP_FileData(void);
#endif
