#ifndef _PAL_GPIO_H_
#define _PAL_GPIO_H_

struct gpio_resource {
    unsigned gpio_group;
    unsigned gpio_pin;
};

struct gpio_lapi {
    int (*yodalite_gpio_request)(unsigned gpio_group, unsigned gpio_pin);
    int (*yodalite_gpio_free)(unsigned gpio_nr);
    int (*yodalite_gpio_direction_input)(unsigned gpio_nr);
    int (*yodalite_gpio_direction_output)(unsigned gpio_nr, int value);
    int (*yodalite_gpio_get_value)(unsigned gpio_nr);
    int (*yodalite_gpio_set_value)(unsigned gpio_nr, int value);
    int (*yodalite_gpio_to_irq)(unsigned gpio_nr);
    int (*yodalite_rawgpio_direction_input)(unsigned gpio_group, unsigned gpio_pin);
    int (*yodalite_rawgpio_direction_output)(unsigned gpio_group, unsigned gpio_pin, int value);
    int (*yodalite_rawgpio_get_value)(unsigned gpio_group, unsigned gpio_pin);
    int (*yodalite_rawgpio_set_value)(unsigned gpio_group, unsigned gpio_pin, int value);
    int (*yodalite_rawgpio_to_irq)(unsigned gpio_group, unsigned gpio_pin,unsigned mode,unsigned pull);
};

extern int pal_gpio_request(unsigned gpio_group, unsigned gpio_pin);

extern int pal_gpio_free(unsigned gpio_nr);

extern int pal_gpio_direction_input(unsigned gpio_nr);

extern int pal_gpio_direction_output(unsigned gpio_nr, int value);

extern int pal_gpio_get_value(unsigned gpio_nr);

extern int pal_gpio_set_value(unsigned gpio_nr, int value);

extern int pal_gpio_to_irq(unsigned gpio_nr);

extern int pal_rawgpio_direction_input(unsigned gpio_group, unsigned gpio_pin);

extern int pal_rawgpio_direction_output(unsigned gpio_group, unsigned gpio_pin, int value);

extern int pal_rawgpio_get_value(unsigned gpio_group, unsigned gpio_pin);

extern int pal_rawgpio_set_value(unsigned gpio_group, unsigned gpio_pin, int value);

extern int pal_rawgpio_to_irq(unsigned gpio_group, unsigned gpio_pin,unsigned mode,unsigned pull);

extern int pal_gpio_init(struct gpio_lapi *platform_gpio_lapi);
#endif	/*_PAL_GPIO_H_*/
