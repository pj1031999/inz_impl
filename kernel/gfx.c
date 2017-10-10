#include <vc_mbox.h>
#include <gfx.h>
#include <font.h>
#include <klibc.h>

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

static GFX_INIT_REQUEST init_request;

window_t screen;

window_t *gfx_set_videomode(unsigned w, unsigned h) {
  init_request.width = init_request.buffer_width = w;
  init_request.height = init_request.buffer_height = h;
  init_request.buffer = 0;
  init_request.pitch = init_request.x = init_request.y =
      init_request.buffer_size = 0;
  init_request.bit_depth = 32;

  vc_mbox_send((uint32_t)&init_request, 1);
  uint32_t reply = vc_mbox_recv(1);

  if (reply != 0)
    return 0;

  screen = (window_t){
    .x = 0,
    .y = 0,
    .width = w,
    .height = h,
    .stride = w,
    .fg_col = color(255,255,255),
    .bg_col = color(0,0,0),
    .pixels = init_request.buffer 
  };

  return &screen;
}

bool gfx_window(window_t *main, window_t *win,
                unsigned x, unsigned y, unsigned width, unsigned height) {
  *win = *main;

  win->x += x;
  win->y += y;
  win->width = width;
  win->height = height;

  if (win->x > main->x + main->width) {
    win->width = 0;
    return false;
  }

  if (win->y > main->y + main->height) {
    win->height = 0;
    return false;
  }

  if (win->x + win->width > main->x + main->width)
    win->width = main->x + main->width - win->x;

  if (win->y + win->height > main->y + main->height)
    win->height = main->x + main->height - win->x;

  return true;
}

void gfx_put_pixel(window_t *win, unsigned x, unsigned y, color_t color) {
  if (x >= win->width)
    return;
  if (y >= win->height)
    return;
  x += win->x;
  y += win->y;
  win->pixels[win->stride * y + x] = color;
}

uint32_t gfx_get_pixel(window_t *win, unsigned x, unsigned y) {
  if (x >= win->width)
    return 0;
  if (y >= win->height)
    return 0;
  x += win->x;
  y += win->y;
  return win->pixels[win->stride * y + x];
}

void gfx_rect_draw(window_t *win, unsigned x, unsigned y,
                   unsigned w, unsigned h, color_t color) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      gfx_put_pixel(win, x + i, y + j, color);
    }
  }
}

void gfx_rect_move(window_t *win, unsigned w, unsigned h,
                   unsigned sx, unsigned sy, unsigned dx, unsigned dy) {
  /* TODO clipping */
  if (sy > dy) {
    for (int y = 0; y < h; y++) {
      uint32_t *src = win->pixels + win->stride * (sy + y) + sx;
      uint32_t *dst = win->pixels + win->stride * (dy + y) + dx;
      memmove(dst, src, w * sizeof(uint32_t));
    }
  }
}

void gfx_put_char(window_t *win, font_t *font, unsigned x, unsigned y, int c) {
  font_char_t *fc = font->map;

  while (fc->code != FONT_CODE_LAST && fc->code != c)
    fc++;

  uint8_t *data = fc->data;
  for (int j = 0; j < font->height; j++) {
    uint8_t row = data[j];
    for (int i = 0; i < 8; i++) {
      uint32_t color = (row & __BIT(7 - i)) ? win->fg_col : win->bg_col;
      gfx_put_pixel(win, x + i, y + j, color);
    }
  }
}
