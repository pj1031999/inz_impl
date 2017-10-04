#ifndef MBOX_H
#define MBOX_H

#include <types.h>

void mbox_set(unsigned cpu, unsigned mbox, uint32_t mask);
void mbox_clr(unsigned cpu, unsigned mbox, uint32_t mask);
uint32_t mbox_recv(unsigned cpu, unsigned mbox);
void mbox_send(unsigned cpu, unsigned mbox, uint32_t value);

#endif
