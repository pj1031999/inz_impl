#ifndef GFX_CONS_H
#define GFX_CONS_H

typedef struct cons cons_t;
typedef struct font font_t;
typedef struct window window_t;

cons_t *make_gfx_cons(window_t *win, font_t *font);

#endif
