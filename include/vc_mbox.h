#ifndef VC_MBOX_H
#define VC_MBOX_H

#include <types.h>

void vc_mbox_send(uint32_t msg, unsigned chan);
uint32_t vc_mbox_recv(unsigned chan);

#endif /* !VC_MBOX_H */
