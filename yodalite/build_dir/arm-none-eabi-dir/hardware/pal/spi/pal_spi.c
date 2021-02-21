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
#include <hardware/pal_spi.h>

static struct spi_lapi pal_spi_lapi = {0};

int pal_spi_init(struct spi_lapi *platform_spi_lapi)
{
    if(!platform_spi_lapi)
        return -STATUS_EINVAL;
    pal_spi_lapi.yodalite_spi_bus_init = platform_spi_lapi->yodalite_spi_bus_init;
    pal_spi_lapi.yodalite_spi_read = platform_spi_lapi->yodalite_spi_read;
    pal_spi_lapi.yodalite_spi_write = platform_spi_lapi->yodalite_spi_write;

    return STATUS_OK;
}

int pal_spi_bus_init(struct spi_resource *pspi_res)
{
    int ret = -STATUS_EIO;
    if(!pspi_res)
        return -STATUS_EINVAL;
    if(pal_spi_lapi.yodalite_spi_bus_init)
        ret = pal_spi_lapi.yodalite_spi_bus_init(pspi_res);
    return ret;
}

int pal_spi_read(unsigned spi_id, unsigned char *buf, unsigned len)
{
    int ret = -STATUS_EIO;
    if(pal_spi_lapi.yodalite_spi_read)
        ret = pal_spi_lapi.yodalite_spi_read(spi_id, buf, len);
    return ret;
}

int pal_spi_write(unsigned spi_id, unsigned char *buf, unsigned len)
{
    int ret = -STATUS_EIO;
    if(pal_spi_lapi.yodalite_spi_write)
        ret = pal_spi_lapi.yodalite_spi_write(spi_id, buf, len);
    return ret;
}

