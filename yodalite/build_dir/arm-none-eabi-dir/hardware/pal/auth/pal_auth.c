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
#include <hardware/pal_auth.h>

static struct auth_lapi pal_auth_lapi = {0};

int pal_auth_init(struct auth_lapi *platform_auth_lapi)
{
    if(!platform_auth_lapi)
        return -STATUS_EINVAL;
    pal_auth_lapi.yodalite_auth_get_sn = platform_auth_lapi->yodalite_auth_get_sn;
    pal_auth_lapi.yodalite_auth_get_chipid = platform_auth_lapi->yodalite_auth_get_chipid;
    pal_auth_lapi.yodalite_auth_get_signature = platform_auth_lapi->yodalite_auth_get_signature;
    pal_auth_lapi.yodalite_auth_get_pubkey = platform_auth_lapi->yodalite_auth_get_pubkey;

    return STATUS_OK;
}

int pal_auth_get_sn(unsigned char *sn)
{
    int ret = -STATUS_EACCES;

    if(pal_auth_lapi.yodalite_auth_get_sn)
        ret = pal_auth_lapi.yodalite_auth_get_sn(sn);

    return ret;
}

int pal_auth_get_chipid(unsigned char *chipid)
{
    int ret = -STATUS_EACCES;

    if(pal_auth_lapi.yodalite_auth_get_chipid)
        ret = pal_auth_lapi.yodalite_auth_get_chipid(chipid);

    return ret;
}

int pal_auth_get_signature(unsigned char *signature)
{
    int ret = -STATUS_EACCES;

    if(pal_auth_lapi.yodalite_auth_get_signature)
        ret = pal_auth_lapi.yodalite_auth_get_signature(signature);

    return ret;
}

int pal_auth_get_pubkey(struct rsapubkey *pubkey)
{
    int ret = -STATUS_EACCES;

    if(pal_auth_lapi.yodalite_auth_get_pubkey)
        ret = pal_auth_lapi.yodalite_auth_get_pubkey(pubkey);

    return ret;
}

