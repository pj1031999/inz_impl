#ifndef RPI_IRQ_H
#define RPI_IRQ_H

#include <dev/bcm2836reg.h>

typedef void (*irq_handler_t)(unsigned irq);

void bcm2835_irq_init(void);
void bcm2835_irq_register(unsigned irq, irq_handler_t handler);
void bcm2835_irq_enable(unsigned irq);
void bcm2835_irq_disable(unsigned irq);

void bcm2836_local_irq_init(void);
void bcm2836_local_irq_register(unsigned irq, irq_handler_t handler);
void bcm2836_local_irq_enable(unsigned irq);
void bcm2836_local_irq_disable(unsigned irq);

#endif /* !RPI_IRQ_H */
