#include <stdint.h>

#include <cdefs.h>
#include <bcm2836reg.h>
#include <mmio.h>
#include <gfx.h>

enum {
  ARMMBOX_BASE = BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_ARMMBOX_BASE),

  GPU_READ   = (ARMMBOX_BASE + 0x00),
  GPU_POLL   = (ARMMBOX_BASE + 0x10),
  GPU_SENDER = (ARMMBOX_BASE + 0x14),
  GPU_STATUS = (ARMMBOX_BASE + 0x18),
  GPU_CONFIG = (ARMMBOX_BASE + 0x1C),
  GPU_WRITE  = (ARMMBOX_BASE + 0x20),
};

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t pitch;
  uint32_t bit_depth;
  uint32_t x;
  uint32_t y;
  void *buffer;
  uint32_t buffer_size;
} GFX_INIT_REQUEST __attribute__((aligned(16)));

static __noinline void send_mail(uint32_t message, uint32_t box) {
  while (mmio_read(GPU_STATUS) & 0x80000000)
    ;

  mmio_write(GPU_WRITE, (message & 0xFFFFFFF0) | (box & 0xF));
}

static __noinline uint32_t recv_mail(uint32_t box) {
  uint32_t value;
  box &= 0xF;
  while (1) {
    while (mmio_read(GPU_STATUS) & 0x40000000)
      ;
    value = mmio_read(GPU_READ);
    if ((value & 0xF) == box) {
      break;
    }
  }

  return value & ~0xF;
}

static GFX_INIT_REQUEST init_request;

void *set_gfx_mode(uint32_t w, uint32_t h, uint32_t bpp) {
  init_request.width = init_request.buffer_width = w;
  init_request.height = init_request.buffer_height = h;
  init_request.buffer = 0;
  init_request.pitch = init_request.x = init_request.y =
      init_request.buffer_size = 0;
  init_request.bit_depth = bpp;

  send_mail((uint32_t)&init_request, 1);
  uint32_t reply = recv_mail(1);

  if (reply == 0) {
    return (void *)init_request.buffer;
  }
  return 0;
}
