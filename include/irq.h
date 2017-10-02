#ifndef BCM2836_IRQ_H
#define BCM2836_IRQ_H

#include <stdbool.h>
#include <dev/bcm2836reg.h>

void bcm2835_irq_enable(unsigned irq);
void bcm2835_irq_disable(unsigned irq);
bool bcm2835_irq_pending_p(unsigned irq);

void bcm2836_local_timer_irq_enable(unsigned cpu, unsigned irq);
void bcm2836_local_timer_irq_disable(unsigned cpu, unsigned irq);
bool bcm2836_local_irq_pending_p(unsigned cpu, unsigned irq);

#endif
