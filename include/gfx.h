#ifndef GFX_H
#define GFX_H

#include <types.h>

typedef struct font font_t;
typedef uint32_t color_t;

#define color(r,g,b) \
  (color_t)((((r) & 255) << 16) | (((g) & 255) << 8) | ((b) & 255))

typedef struct window {
  unsigned x, y;
  unsigned width, height;
  unsigned stride;
  color_t fg_col, bg_col;
  color_t *pixels;
} window_t;

window_t *gfx_set_videomode(unsigned w, unsigned h);
bool gfx_window(window_t *main, window_t *win,
                unsigned x, unsigned y, unsigned w, unsigned h);
void gfx_put_pixel(window_t *win, unsigned x, unsigned y, color_t color);
void gfx_put_char(window_t *win, font_t *font, unsigned x, unsigned y, int c);
void gfx_rect_move(window_t *win, unsigned w, unsigned h,
                   unsigned sx, unsigned sy, unsigned dx, unsigned dy);
void gfx_rect_draw(window_t *win, unsigned x, unsigned y,
                   unsigned w, unsigned h, color_t color);

static inline void gfx_set_bg_col(window_t *win, color_t color) {
  win->bg_col = color;
}

#endif /* !GFX_H */
