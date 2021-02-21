#ifndef _PAL_IRQ_H_
#define _PAL_IRQ_H_

typedef int (*irq_handler_t) (int irq);

struct irq_lapi {
    int (*yodalite_irq_request)(unsigned int irq, irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id);
    int (*yodalite_irq_enable)(unsigned int irq);
    int (*yodalite_irq_disable)(unsigned int irq);
};

#define ALL_IRQ    (-1)

extern int pal_irq_init(struct irq_lapi *platform_irq_lapi);
extern int pal_irq_request(unsigned int irq, irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id);
extern int pal_irq_enable(unsigned int irq);
extern int pal_irq_disable(unsigned int irq);

#endif  /*_PAL_IRQ_H_*/

