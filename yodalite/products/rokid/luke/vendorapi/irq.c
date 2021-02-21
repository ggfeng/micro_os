#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <hardware/platform.h>
#include <hardware/pal_irq.h>

int stm32_irq_request(unsigned int irq,irq_handler_t handler,unsigned long irqflags,const char *devname,void *dev_id)
{
	volatile long int *irq_hand=0x00000040;
	*(irq_hand+4*irq)=handler;	

}
int stm32_irq_enable(unsigned int irq)
{
	HAL_NVIC_EnableIRQ(irq);
	return 1;
}
int stm32_irq_disable(unsigned int irq)
{
	HAL_NVIC_DisableIRQ(irq);
	return 1;
}

struct irq_lapi pal_irq_lapi = {
		.yodalite_irq_request = stm32_irq_request,
		.yodalite_irq_enable  = stm32_irq_enable,
		.yodalite_irq_disable = stm32_irq_disable,
};

