#include <arm/mbox.h>
#include <dev/bcm2836reg.h>
#include <mmio.h>

#define MAILBOX_SET_N(n, m)                                                    \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX_SETN(n, m))
#define MAILBOX_CLR_N(n, m)                                                    \
  (BCM2836_ARM_LOCAL_BASE + BCM2836_LOCAL_MAILBOX_CLRN(n, m))

void mbox_set(unsigned cpu, unsigned mbox, uint32_t mask) {
  mmio_write(MAILBOX_SET_N(cpu, mbox), mask);
}

void mbox_clr(unsigned cpu, unsigned mbox, uint32_t mask) {
  mmio_write(MAILBOX_CLR_N(cpu, mbox), mask);
}

uint32_t mbox_recv(unsigned cpu, unsigned mbox) {
  return mmio_read(MAILBOX_CLR_N(cpu, mbox));
}

void mbox_send(unsigned cpu, unsigned mbox, uint32_t value) {
  mbox_clr(cpu, mbox, -1);
  mbox_set(cpu, mbox, value);
}
