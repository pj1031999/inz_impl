#ifndef FONT_H
#define FONT_H

#include <types.h>

typedef struct font_char {
  uint32_t code;
  uint8_t *data;
} font_char_t;

#define FONT_CODE_LAST 0xfffd

typedef struct font {
  unsigned width;
  unsigned height;
  font_char_t map[];
} font_t;

#endif /* !FONT_H */
