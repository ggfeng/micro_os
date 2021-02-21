#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <hardware/platform.h>
#include <hardware/pal_gpio.h>


#include <main.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal.h>

int stm32_rawgpio_direction_input(unsigned gpio_group,unsigned gpio_pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin =   1<<gpio_pin;		//LT_RST
  	GPIO_InitStruct.Mode =  GPIO_MODE_INPUT   ;///
  	GPIO_InitStruct.Pull =  GPIO_PULLUP;
	switch(gpio_group)
	{
		case 0:
			__HAL_RCC_GPIOA_CLK_ENABLE();
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
		break;
		case 1:
			__HAL_RCC_GPIOB_CLK_ENABLE();
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
		break;
		case 2:
			__HAL_RCC_GPIOC_CLK_ENABLE();
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
		break;
		case 3:
			__HAL_RCC_GPIOD_CLK_ENABLE();
			HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);	
		break;
		case 4:
			__HAL_RCC_GPIOE_CLK_ENABLE();
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);	
		break;
		case 5:
			__HAL_RCC_GPIOH_CLK_ENABLE();
			HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);	
		break;
	}
	return 1;
	
}

int stm32_rawgpio_direction_output(unsigned gpio_group,unsigned gpio_pin,int value)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin =   1<<gpio_pin;            //LT_RST
        GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP   ;///
        GPIO_InitStruct.Pull =  GPIO_PULLUP;
        switch(gpio_group)
        {
                case 0:
                        __HAL_RCC_GPIOA_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
                break;
                case 1:
                        __HAL_RCC_GPIOB_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
                break;
                case 2:
                        __HAL_RCC_GPIOC_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 
                break;
                case 3:
                        __HAL_RCC_GPIOD_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
                break;
                case 4:
                        __HAL_RCC_GPIOE_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
                break;
                case 5:
                        __HAL_RCC_GPIOH_CLK_ENABLE();
                        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
                break;
        }
        return 1;

}

int stm32_rawgpio_get_value(unsigned gpio_group,unsigned gpio_pin)
{
	switch(gpio_group)
	{
		case 0:
			return ((GPIOA->IDR)&(~(1<<gpio_pin)))>>gpio_pin;			
		break;
		case 1:
			return ((GPIOB->IDR)&(~(1<<gpio_pin)))>>gpio_pin;
		break;
		case 2:
			return ((GPIOC->IDR)&(~(1<<gpio_pin)))>>gpio_pin;
		break;
		case 3:
			return ((GPIOD->IDR)&(~(1<<gpio_pin)))>>gpio_pin;
		break;
		case 4:
			return ((GPIOE->IDR)&(~(1<<gpio_pin)))>>gpio_pin;
		break;
		case 5:
			return ((GPIOH->IDR)&(~(1<<gpio_pin)))>>gpio_pin;
		break;
	}

	return 0xf0;
}

int stm32_rawgpio_set_value(unsigned gpio_group,unsigned gpio_pin,int value)
{

	switch(gpio_group)
	{
		case 0:
			if(value)
			GPIOA->ODR|= (1<<gpio_pin);
			else
			GPIOA->ODR&=~(1<<gpio_pin);
		break;
		case 1:
                        if(value)
                        GPIOB->ODR|= (1<<gpio_pin);
                        else
                        GPIOB->ODR&=~(1<<gpio_pin);
		break;
		case 2:
                        if(value)
                        GPIOC->ODR|= (1<<gpio_pin);
                        else
                        GPIOC->ODR&=~(1<<gpio_pin);
		break;
		case 3:
                        if(value)
                        GPIOD->ODR|= (1<<gpio_pin);
                        else
                        GPIOD->ODR&=~(1<<gpio_pin);
		break;
		case 4:
                        if(value)
                        GPIOE->ODR|= (1<<gpio_pin);
                        else
                        GPIOE->ODR&=~(1<<gpio_pin);
		break;
		case 5:
                        if(value)
                        GPIOH->ODR|= (1<<gpio_pin);
                        else
                        GPIOH->ODR&=~(1<<gpio_pin);
		break;

	}
	return 1;
}

int stm32_rawgpio_to_irq(unsigned gpio_group,unsigned gpio_pin,unsigned mode,unsigned pull)
{
	unsigned char exti;
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = 1<<gpio_pin;
        GPIO_InitStruct.Mode = mode;
        GPIO_InitStruct.Pull = pull;
	if(gpio_pin<5)
       	     exti=6+gpio_pin;
	else if(gpio_pin<=9)
            exti=23;
        else if(gpio_pin<=15)
            exti=40;
        else
            return 0;
        HAL_NVIC_SetPriority(exti, 10, 2);
        HAL_NVIC_EnableIRQ(exti);
	switch(gpio_group)
	{
		case 0:
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		break;
		case 1:
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		break;
		case 2:
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		break;
		case 3:
			HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
		break;
		case 4:
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
		break;
		case 5:
			HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
		break;
	}
	return 1;
}



struct gpio_lapi pal_gpio_lapi={
		.yodalite_rawgpio_direction_input = stm32_rawgpio_direction_input,
		.yodalite_rawgpio_direction_output  = stm32_rawgpio_direction_output,
		.yodalite_rawgpio_get_value = stm32_rawgpio_get_value,
		.yodalite_rawgpio_set_value  = stm32_rawgpio_set_value,
		.yodalite_rawgpio_to_irq  = stm32_rawgpio_to_irq,
};
