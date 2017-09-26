#ifndef BCM2836_H
#define BCM2836_H

#include <stdbool.h>
#include <bcm2836reg.h>
#include <bcm2836.h>

void bcm2836_local_timer_irq_enable(unsigned cpunum, unsigned irqnum);
bool bcm2836_local_irq_pending_p(unsigned cpunum, unsigned irqnum);

#endif
