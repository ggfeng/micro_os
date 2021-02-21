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
#include <hardware/pal_usb.h>

static struct usb_lapi pal_usb_lapi = {0};

int pal_usb_init(struct usb_lapi *platform_usb_lapi)
{
    if(!platform_usb_lapi)
        return -1;
    pal_usb_lapi.yodalite_usb_bus_init = platform_usb_lapi->yodalite_usb_bus_init;
    pal_usb_lapi.yodalite_usb_receive = platform_usb_lapi->yodalite_usb_receive;
    pal_usb_lapi.yodalite_usb_transmit = platform_usb_lapi->yodalite_usb_transmit;
    return 0;
}

/**
* @brief  Initializes the device stack and load the class driver
* @param  usb_id: Usb device id
* @param  usb_descriptor: Usb Descriptor data address
* @param  pep_addr: the pointer of the usb end point address, get the end point address  for this usb descriptor channel through this parameter
* @retval USBD status
*/
int pal_usb_bus_init(struct usb_resource *pusb_res, uint8_t *pep_addr)
{
    int ret = -1;

    if(!pusb_res || !pep_addr)
        return -STATUS_EINVAL;
    if(pal_usb_lapi.yodalite_usb_bus_init)
        ret = pal_usb_lapi.yodalite_usb_bus_init(pusb_res, pep_addr);

    return ret;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  usb_id: Usb Device id
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
int pal_usb_receive(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
    int ret = -1;

    if(pal_usb_lapi.yodalite_usb_receive)
        ret = pal_usb_lapi.yodalite_usb_receive(usb_id, ep_addr, pbuf, size);

    return ret;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  usb_id: Usb Device id
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
int pal_usb_transmit(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
    int ret = -1;

    if(pal_usb_lapi.yodalite_usb_transmit)
        ret = pal_usb_lapi.yodalite_usb_transmit(usb_id, ep_addr, pbuf, size);

    return ret;
}


