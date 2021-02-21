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

#include <hardware/platform.h>
#include <hardware/pal_uart.h>

static struct uart_lapi pal_uart_lapi = {0};

int pal_uart_init(struct uart_lapi *platform_uart_lapi)
{
    if(!platform_uart_lapi)
        return -STATUS_EINVAL;
    pal_uart_lapi.yodalite_uart_bus_init = platform_uart_lapi->yodalite_uart_bus_init;
    pal_uart_lapi.yodalite_uart_read = platform_uart_lapi->yodalite_uart_read;
    pal_uart_lapi.yodalite_uart_write = platform_uart_lapi->yodalite_uart_write;
    return STATUS_OK;
}

int pal_uart_bus_init(struct uart_resource *puart_res)
{
    int ret = -STATUS_EIO;
    if(!puart_res)
        return -STATUS_EINVAL;
    if(pal_uart_lapi.yodalite_uart_bus_init)
        ret = pal_uart_lapi.yodalite_uart_bus_init(puart_res);
    return ret;
}

int pal_uart_read(unsigned uart_id, unsigned char *buf, unsigned len)
{
    int ret = -STATUS_EIO;
    if(pal_uart_lapi.yodalite_uart_read)
        ret = pal_uart_lapi.yodalite_uart_read(uart_id, buf, len);
    return ret;
}

int pal_uart_write(unsigned uart_id, unsigned char *buf, unsigned len)
{
    int ret = -STATUS_EIO;
    if(pal_uart_lapi.yodalite_uart_write)
        ret = pal_uart_lapi.yodalite_uart_write(uart_id, buf, len);
    return ret;
}


