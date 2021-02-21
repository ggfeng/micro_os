#ifndef _PAL_UART_H_
#define _PAL_UART_H_

struct uart_resource {
    unsigned uart_id;
    unsigned baud_rate;
};

struct uart_lapi {
    int (*yodalite_uart_bus_init)(struct uart_resource *puart_res);
    int (*yodalite_uart_read)(unsigned uart_id, unsigned char *buf, unsigned len);
    int (*yodalite_uart_write)(unsigned uart_id, unsigned char *buf, unsigned len);
};

extern int pal_uart_init(struct uart_lapi *platform_uart_lapi);
extern int pal_uart_bus_init(struct uart_resource *puart_res);
extern int pal_uart_read(unsigned uart_id, unsigned char *buf, unsigned len);
extern int pal_uart_write(unsigned uart_id, unsigned char *buf, unsigned len);

#endif  /*_PAL_UART_H_*/

