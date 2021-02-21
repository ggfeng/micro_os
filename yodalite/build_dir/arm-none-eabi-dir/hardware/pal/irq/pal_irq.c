/*
 * (C) Copyright 2019 Rokid Corp.
 * Zhu Bin <bin.zhu@rokid.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <hardware/platform.h>
#include <hardware/pal_irq.h>

static struct irq_lapi pal_irq_lapi = {0};

int pal_irq_init(struct irq_lapi *platform_irq_lapi)
{
    if(!platform_irq_lapi)
        return -STATUS_EINVAL;
    pal_irq_lapi.yodalite_irq_request = platform_irq_lapi->yodalite_irq_request;
    pal_irq_lapi.yodalite_irq_enable = platform_irq_lapi->yodalite_irq_enable;
    pal_irq_lapi.yodalite_irq_disable = platform_irq_lapi->yodalite_irq_disable;

    return STATUS_OK;
}

 /**
 * @brief  request an irq handler for a specific interrupt
 * @param  irq: Interrupt line to allocate
 * @param  handler: Function to be called when the IRQ occurs
 * @param irqflags: Interrupt type flags
 * @param devname: An ascii name for the claiming device
 * @param dev_id: A cookie passed back to the handler function
 * @retval STATUS_OK for function running well and other value for function running fault
 */
int pal_irq_request(unsigned int irq, irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id)
{
    int ret = -STATUS_EACCES;

    if(pal_irq_lapi.yodalite_irq_request)
        ret = pal_irq_lapi.yodalite_irq_request(irq, handler, irqflags, devname, dev_id);

    return ret;
}

/**
* @brief enable a specific interrupt, or enable all interrupts if irq is ALL_IRQ
* @param  irq: Interrupt line to allocate
* @retval STATUS_OK for function running well and other value for function running fault
*/
int pal_irq_enable(unsigned int irq)
{
    int ret = -STATUS_EACCES;

    if(pal_irq_lapi.yodalite_irq_enable)
        ret = pal_irq_lapi.yodalite_irq_enable(irq);

    return ret;
}

/**
* @brief disable a specific interrupt, or disable all interrupts if irq is ALL_IRQ
* @param  irq: Interrupt line to allocate
* @retval STATUS_OK for function running well and other value for function running fault
*/
int pal_irq_disable(unsigned int irq)
{
    int ret = -STATUS_EACCES;

    if(pal_irq_lapi.yodalite_irq_disable)
        ret = pal_irq_lapi.yodalite_irq_disable(irq);

    return ret;
}

