#ifndef GFX_H
#define GFX_H

#include <types.h>
#include <rpi/vc_prop.h>

typedef struct font font_t;
typedef uint32_t color_t;
// volatile unsigned int _mail_buffer[256] __attribute__((aligned (16)));
extern int _mail_buffer[256];

#define color(r, g, b)                                                         \
  (color_t)((((r)&255) << 16) | (((g)&255) << 8) | ((b)&255))

typedef struct window {
  unsigned x, y;
  unsigned width, height;
  unsigned stride;
  color_t fg_col, bg_col;
  color_t *pixels;
} window_t;

window_t *gfx_set_videomode(unsigned w, unsigned h);
bool gfx_window(window_t *main, window_t *win, unsigned x, unsigned y,
                unsigned w, unsigned h);
void gfx_put_pixel(window_t *win, unsigned x, unsigned y, color_t color);
void gfx_put_char(window_t *win, font_t *font, unsigned x, unsigned y, int c);
void gfx_rect_move(window_t *win, unsigned w, unsigned h, unsigned sx,
                   unsigned sy, unsigned dx, unsigned dy);
void gfx_rect_draw(window_t *win, unsigned x, unsigned y, unsigned w,
                   unsigned h, color_t color);

static inline void gfx_set_bg_col(window_t *win, color_t color) {
  win->bg_col = color;
}

struct vcprop_tag_allocbuf *
get_response_tag_allocbuf(const unsigned int mailbuffer[], const uint32_t c);

uint32_t set_mailbufer(unsigned int mailbuffer[], unsigned w, unsigned h);

#define fill(vcprop_tag_name)                                                  \
  size_t fill_##vcprop_tag_name(volatile unsigned int mailbuffer[], size_t c,  \
                                struct vcprop_tag_name val) {                  \
    *(struct vcprop_tag_name *)(&mailbuffer[c]) = val;                         \
    return sizeof(struct vcprop_tag_name) / sizeof mailbuffer[0];              \
  }

#endif /* !GFX_H */
