#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <hardware/platform.h>
#include <hardware/pal_usb.h>
#include "usbd_audio.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usb_device.h"
int stm32_usb_bus_init(struct usb_resource *pusb_re,uint8_t *pep_addr)
{
	MX_USB_DEVICE_Init();

}
int stm32_usb_receive(unsigned usb_id,uint8_t ep_addr,uint8_t *pbuf,uint16_t size)
{
	USBD_LL_PrepareReceive(&hUsbDeviceFS,ep_addr,pbuf,size);
}
int stm32_usb_transmit(unsigned usb_id,uint8_t ep_addr,uint8_t *pbuf,uint16_t size)
{
	USBD_LL_Transmit(&hUsbDeviceFS,epddr,pbuf,size);
}

struct usb_lapi pal_usb_lapi = {
		pal_usb_bus_init = stm32_usb_bus_init,
		pal_usb_receive = stm32_usb_receive,
		pal_usb_transmit = stm32_usb_transmit,
};

