/**
 * Copyright 2017 Hillcrest Laboratories, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License and 
 * any applicable agreements you may have with Hillcrest Laboratories, Inc.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "string.h"
#include "sh2_hal.h"
#include "sh2_err.h"
#include "shtpI2CHal.h"
#include "mxconstants.h"
#include "hardware/bno08x_control.h"
#include <hardware/platform.h>


Sh2Hal_t sh2Hal;
volatile bool intnAsserted;
unsigned char hi2c;
static void bootn0(bool state)
{
	pal_rawgpio_direction_output(BOOTN_Pin,BOOTN_GPIO_Port,state);
}

static void rstn0(bool state)
{
	pal_rawgpio_direction_output(RSTN_Pin,RSTN_GPIO_Port,state);
}
int sh2_hal_block(void)
{
    while(sh2Hal.blockSem)
    {
        halTask();
    }
    sh2Hal.blockSem = true;
    return SH2_OK;
}

int sh2_hal_unblock(void)
{
    sh2Hal.blockSem = false;
    return SH2_OK;
}


// Initialize SH-2 HAL subsystem
void sh2_hal_init(unsigned char _hi2c) 
{
    // Store reference to I2C peripheral
    hi2c = _hi2c;   
    // Initialize SH2 HAL data structure
    memset(&sh2Hal, 0, sizeof(sh2Hal));
    sh2Hal.rstn = rstn0;
    sh2Hal.bootn = bootn0;
    // Put SH2 device in reset
    sh2Hal.rstn(false);  // Hold in reset
    sh2Hal.bootn(true);  // SH-2, not DFU
    sh2Hal.blockSem = true;
}

void usleep(unsigned short int time)
{
	unsigned char i;
	for(;time>0;time--)
	 for(i=121;i>0;i--)
	 {
	 	
	 }
}

int sh2_hal_reset(bool dfuMode,
                  sh2_rxCallback_t *onRx,
                  void *cookie)
{
    // Store params for later reference
    sh2Hal.onRxCookie = cookie;
    sh2Hal.onRx = onRx;    
    // Set addr to use in this mode
    sh2Hal.addr = dfuMode ? ADDR_DFU_0<<1 : ADDR_SH2_0<<1;
    // Assert reset
    sh2Hal.rstn(0);
    // Set BOOTN according to dfuMode
    sh2Hal.bootn(dfuMode ? 0 : 1);
    // Wait for reset to take effect
    //usleep(RESET_DELAY); 
    usleep(100);
    // Deassert reset
    sh2Hal.rstn(1);
    
	intnAsserted = false;
    intnAsserted = true;///
    // If reset into DFU mode, wait until bootloader should be ready
    if (dfuMode) {
       // usleep(DFU_BOOT_DELAY);
       usleep(1000);
    }
    return SH2_OK;
    
}


int sh2_hal_tx(uint8_t *pData, uint32_t len)
{
    // Do nothing if len is zero
    if (len == 0) {
        return SH2_OK;
    }
    
    // Do tx, and return when done
    return pal_i2c_block_write(hi2c, sh2Hal.addr,0x00,0x01, pData, len);
}


int sh2_hal_rx(uint8_t *pData, uint32_t len)
{
    // Do nothing if len is zero
    if (len == 0) {
        return SH2_OK;
    }
    
    // do rx and return when done
    return pal_i2c_block_read(hi2c, sh2Hal.addr,0x00,0x01,pData, len);
}



void halTask()
{
  // 	intnAsserted = true;
    if (false == intnAsserted)
        return;
    intnAsserted = false;
    static unsigned int readLen = 0;
    static unsigned int cargoLen = 0;
    // If no RX callback registered, don't bother trying to read
    if (sh2Hal.onRx != 0) {
        // Compute read length
        readLen = sh2Hal.rxRemaining;
        if (readLen < SHTP_HEADER_LEN) {
            // always read at least the SHTP header
            readLen = SHTP_HEADER_LEN;
        }
        if (readLen > SH2_HAL_MAX_TRANSFER) {
            // limit reads to transfer size
            readLen = SH2_HAL_MAX_TRANSFER;
        }
        
        // Read i2c
	pal_i2c_block_read(hi2c,sh2Hal.addr,0x00,0x01,sh2Hal.rxBuf,readLen); 
        //HAL_I2C_Master_Receive(hi2c, sh2Hal.addr, sh2Hal.rxBuf, readLen, 100);//100
        cargoLen = ((sh2Hal.rxBuf[1] << 8) + (sh2Hal.rxBuf[0])) & (~0x8000);
        // Re-Evaluate rxRemaining
        if (cargoLen > readLen) {
            // More to read.
            sh2Hal.rxRemaining = (cargoLen - readLen) + SHTP_HEADER_LEN;
        }
        else {
            // All done, next read should be header only.
            sh2Hal.rxRemaining = 0;
        }
        sh2Hal.onRx(sh2Hal.onRxCookie, sh2Hal.rxBuf, readLen, HAL_GetTick() * 50000);
    }	
}
void IMU_IT(uint16_t GPIO_Pin)
{
    // INTN
    if(INTN_Pin == GPIO_Pin) 
		{    
        intnAsserted = true;       
			//	xSemaphoreGive(wakeSensorTask1);
    }
}

