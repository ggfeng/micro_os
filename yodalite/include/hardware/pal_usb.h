#ifndef _PAL_USB_H_
#define _PAL_USB_H_

#include <stdint.h>

struct usb_resource {
    unsigned usb_id;
    unsigned char *usb_descriptor;
};

struct usb_lapi {
    int (*yodalite_usb_bus_init)(struct usb_resource *pusb_res, uint8_t *pep_addr);
    int (*yodalite_usb_receive)(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size);
    int (*yodalite_usb_transmit)(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size);
};

extern int pal_usb_init(struct usb_lapi *platform_usb_lapi);
extern int pal_usb_bus_init(struct usb_resource *pusb_res, uint8_t *pep_addr);
extern int pal_usb_receive(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size);
extern int pal_usb_transmit(unsigned usb_id, uint8_t ep_addr, uint8_t *pbuf, uint16_t size);

#endif  /*_PAL_USB_H_*/

