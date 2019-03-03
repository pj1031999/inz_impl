#ifndef RPI_VC_MBOX_H
#define RPI_VC_MBOX_H

#include <types.h>

/* https://github.com/raspberrypi/firmware/wiki/Mailboxes */
#define VC_CHAN_PM 0
#define VC_CHAN_FB 1
#define VC_CHAN_VUART 2
#define VC_CHAN_VCHIQ 3
#define VC_CHAN_LEDS 4
#define VC_CHAN_BUTTONS 5
#define VC_CHAN_TOUCHSCR 6
#define VC_CHAN_ARM2VC 8
#define VC_CHAN_VC2ARM 9

void vc_mbox_send(unsigned long msg, unsigned chan);
uint32_t vc_mbox_recv(unsigned chan);

#endif /* !RPI_VC_MBOX_H */
