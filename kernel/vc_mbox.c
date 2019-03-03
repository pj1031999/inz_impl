#include <mmio.h>
#include <dev/bcm2836reg.h>
#include <rpi/vc_mbox.h>
#include <klibc.h>

#define ARMMBOX_BASE BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_ARMMBOX_BASE)

#define MBOX_READ   0x00
#define MBOX_WRITE  0x00
#define MBOX_POLL   0x10
#define MBOX_ID     0x14
#define MBOX_STATUS 0x18

#define MBOX0_BASE   ARMMBOX_BASE + 0x00
#define MBOX0_READ   MBOX0_BASE + MBOX_READ
#define MBOX0_WRITE  MBOX0_BASE + MBOX_WRITE
#define MBOX0_POLL   MBOX0_BASE + MBOX_POLL
#define MBOX0_ID     MBOX0_BASE + MBOX_ID
#define MBOX0_STATUS MBOX0_BASE + MBOX_STATUS
#define MBOX0_CFG    MBOX0_BASE + MBOX_READ

#define MBOX1_BASE   ARMMBOX_BASE + 0x20
#define MBOX1_READ   MBOX1_BASE + MBOX_READ
#define MBOX1_WRITE  MBOX1_BASE + MBOX_WRITE
#define MBOX1_POLL   MBOX1_BASE + MBOX_POLL
#define MBOX1_ID     MBOX1_BASE + MBOX_ID
#define MBOX1_STATUS MBOX1_BASE + MBOX_STATUS
#define MBOX1_CFG    MBOX1_BASE + MBOX_READ

#define MBOX_STATUS_FULL  __BIT(31)
#define MBOX_STATUS_EMPTY __BIT(30)

#define MBOX_CHAN(chan) ((chan) & 15)
#define MBOX_DATA(data) ((data) & ~15)
#define MBOX_MSG(data, chan) (MBOX_DATA(data) | MBOX_CHAN(chan))

void vc_mbox_send(unsigned long msg, unsigned chan) {
  assert(MBOX_CHAN(chan) == chan);
  assert(MBOX_CHAN(msg) == 0);

  while (mmio_read(MBOX0_STATUS) & MBOX_STATUS_FULL)
    ;

  mmio_write(MBOX1_WRITE, MBOX_MSG(msg, chan));
}

uint32_t vc_mbox_recv(unsigned chan) {
  assert(MBOX_CHAN(chan) == chan);

  for (;;) {
    while (mmio_read(MBOX0_STATUS) & MBOX_STATUS_EMPTY)
      ;
    uint32_t val = mmio_read(MBOX0_READ);
    if (MBOX_CHAN(val) == chan)
      return MBOX_DATA(val);
  }
}

