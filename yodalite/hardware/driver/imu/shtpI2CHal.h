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

#ifndef __SHTP_I2C_HAL_H
#define __SHTP_I2C_HAL_H
#include "sh2_hal.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h"

#define ADDR_DFU_0 (0x28)
#define ADDR_DFU_1 (0x29)
#define ADDR_SH2_0 (0x4A)
#define ADDR_SH2_1 (0x4B)

#define SHTP_HEADER_LEN (4)

#define DFU_BOOT_DELAY (200) // [mS]
#define RESET_DELAY    (10) // [mS]

typedef struct {
    void (*rstn)(bool);
    void (*bootn)(bool);
    sh2_rxCallback_t *onRx;
    void *onRxCookie;
    uint16_t addr;
    uint8_t rxBuf[SH2_HAL_MAX_TRANSFER];
    uint16_t rxRemaining;
    unsigned char blockSem;
} Sh2Hal_t;

void sh2_hal_init(unsigned char _hi2c);
void halTask(void);
#endif /* __SHTP_I2C_HAL_H */
