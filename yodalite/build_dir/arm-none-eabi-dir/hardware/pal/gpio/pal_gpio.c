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

#include <hardware/pal_gpio.h>

static struct gpio_lapi pal_gpio_lapi = {0};

int pal_gpio_init(struct gpio_lapi * platform_gpio_lapi)
{
    if(!platform_gpio_lapi)
        return -1;
    pal_gpio_lapi.yodalite_gpio_request = platform_gpio_lapi->yodalite_gpio_request;
    pal_gpio_lapi.yodalite_gpio_free = platform_gpio_lapi->yodalite_gpio_free;
    pal_gpio_lapi.yodalite_gpio_direction_input = platform_gpio_lapi->yodalite_gpio_direction_input;
    pal_gpio_lapi.yodalite_gpio_direction_output = platform_gpio_lapi->yodalite_gpio_direction_output;
    pal_gpio_lapi.yodalite_gpio_get_value = platform_gpio_lapi->yodalite_gpio_get_value;
    pal_gpio_lapi.yodalite_gpio_set_value = platform_gpio_lapi->yodalite_gpio_set_value;
    pal_gpio_lapi.yodalite_gpio_to_irq = platform_gpio_lapi->yodalite_gpio_to_irq;
    pal_gpio_lapi.yodalite_rawgpio_direction_input = platform_gpio_lapi->yodalite_rawgpio_direction_input;
    pal_gpio_lapi.yodalite_rawgpio_direction_output = platform_gpio_lapi->yodalite_rawgpio_direction_output;
    pal_gpio_lapi.yodalite_rawgpio_get_value = platform_gpio_lapi->yodalite_rawgpio_get_value;
    pal_gpio_lapi.yodalite_rawgpio_set_value = platform_gpio_lapi->yodalite_rawgpio_set_value;
    pal_gpio_lapi.yodalite_rawgpio_to_irq = platform_gpio_lapi->yodalite_rawgpio_to_irq;

    return 0;
}

int pal_gpio_request(unsigned gpio_group, unsigned gpio_pin)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_request)
	ret = pal_gpio_lapi.yodalite_gpio_request(gpio_group, gpio_pin);

    return ret;
}

int pal_gpio_free(unsigned gpio_nr)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_free)
	ret = pal_gpio_lapi.yodalite_gpio_free(gpio_nr);

    return ret;
}

int pal_gpio_direction_input(unsigned gpio_nr)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_direction_input)
	ret = pal_gpio_lapi.yodalite_gpio_direction_input(gpio_nr);

    return ret;
}

int pal_gpio_direction_output(unsigned gpio_nr, int value)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_direction_output)
	ret = pal_gpio_lapi.yodalite_gpio_direction_output(gpio_nr, value);

    return ret;
}

int pal_gpio_get_value(unsigned gpio_nr)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_get_value)
	ret = pal_gpio_lapi.yodalite_gpio_get_value(gpio_nr);

    return ret;
}

int pal_gpio_set_value(unsigned gpio_nr, int value)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_set_value)
	ret = pal_gpio_lapi.yodalite_gpio_set_value(gpio_nr, value);

    return ret;
}

int pal_gpio_to_irq(unsigned gpio_nr)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_gpio_to_irq)
	ret = pal_gpio_lapi.yodalite_gpio_to_irq(gpio_nr);

    return ret;
}

int pal_rawgpio_direction_input(unsigned gpio_group, unsigned gpio_pin)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_rawgpio_direction_input)
	ret = pal_gpio_lapi.yodalite_rawgpio_direction_input(gpio_group, gpio_pin);

    return ret;
}

int pal_rawgpio_direction_output(unsigned gpio_group, unsigned gpio_pin, int value)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_rawgpio_direction_output)
	ret = pal_gpio_lapi.yodalite_rawgpio_direction_output(gpio_group, gpio_pin, value);

    return ret;
}

int pal_rawgpio_get_value(unsigned gpio_group, unsigned gpio_pin)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_rawgpio_get_value)
	ret = pal_gpio_lapi.yodalite_rawgpio_get_value(gpio_group, gpio_pin);

    return ret;
}

int pal_rawgpio_set_value(unsigned gpio_group, unsigned gpio_pin, int value)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_rawgpio_set_value)
	ret = pal_gpio_lapi.yodalite_rawgpio_set_value(gpio_group, gpio_pin, value);

    return ret;
}

int pal_rawgpio_to_irq(unsigned gpio_group, unsigned gpio_pin,unsigned mode,unsigned pull)
{
    int ret = -1;

    if(pal_gpio_lapi.yodalite_rawgpio_to_irq)
	ret = pal_gpio_lapi.yodalite_rawgpio_to_irq(gpio_group, gpio_pin,mode,pull);

    return ret;
}
