#ifndef  __VENDORAPI_H_
#define  __VENDORAPI_H_
#include <hardware/platform.h>
//#include <hardware/i2c.h>
#include <main.h>
#include "stm32f4xx_hal.h"


extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

extern struct irq_lapi pal_irq_lapi;
extern struct gpio_lapi pal_gpio_lapi;
extern struct i2c_lapi pal_i2c_lapi;
#endif             
