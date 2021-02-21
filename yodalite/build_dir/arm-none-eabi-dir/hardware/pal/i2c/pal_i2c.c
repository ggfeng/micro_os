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
#include <hardware/pal_i2c.h>

static struct i2c_lapi pal_i2c_lapi = {0};

int pal_i2c_init(struct i2c_lapi *platform_i2c_lapi)
{
    if(!platform_i2c_lapi)
        return -1;
    pal_i2c_lapi.yodalite_i2c_bus_init = platform_i2c_lapi->yodalite_i2c_bus_init;
    pal_i2c_lapi.yodalite_i2c_read = platform_i2c_lapi->yodalite_i2c_read;
    pal_i2c_lapi.yodalite_i2c_write = platform_i2c_lapi->yodalite_i2c_write;
    pal_i2c_lapi.yodalite_i2c_block_write = platform_i2c_lapi->yodalite_i2c_block_write;
    pal_i2c_lapi.yodalite_i2c_block_read = platform_i2c_lapi->yodalite_i2c_block_read;

    return 0;
}

int pal_i2c_bus_init(struct i2c_resource *pi2c_res)
{
    int ret = -1;

    if(!pi2c_res)
        ret -STATUS_EINVAL;
    if(pal_i2c_lapi.yodalite_i2c_bus_init)
        ret = pal_i2c_lapi.yodalite_i2c_bus_init(pi2c_res);

    return ret;
}

int pal_i2c_read(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode, unsigned char *pdata,unsigned char len)
{
    int ret = -1;

    if(pal_i2c_lapi.yodalite_i2c_read)
        ret = pal_i2c_lapi.yodalite_i2c_read(i2c_id, client, addr,mode,pdata,len);

    return ret;
}

int pal_i2c_write(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode ,unsigned char *pdata,unsigned char len)
{
    int ret = -1;

    if(pal_i2c_lapi.yodalite_i2c_write)
        ret = pal_i2c_lapi.yodalite_i2c_write(i2c_id, client, addr,mode,pdata,len);

    return ret;
}
int pal_i2c_block_read(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode, unsigned char *pdata,unsigned char len)
{
    int ret = -1;

    if(pal_i2c_lapi.yodalite_i2c_block_read)
        ret = pal_i2c_lapi.yodalite_i2c_block_read(i2c_id, client, addr,mode,pdata,len);

    return ret;
}

int pal_i2c_block_write(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode ,unsigned char *pdata,unsigned char len)
{
    int ret = -1;

    if(pal_i2c_lapi.yodalite_i2c_block_write)
        ret = pal_i2c_lapi.yodalite_i2c_block_write(i2c_id, client, addr,mode,pdata,len);

    return ret;
}

