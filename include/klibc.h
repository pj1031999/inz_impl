#ifndef KLIBC_H
#define KLIBC_H

#include <types.h>

void __assert(const char *expr, const char *file, unsigned line);

#define assert(EXPR)                                                           \
  __extension__({                                                              \
    if (!(EXPR))                                                               \
      __assert(__STRING(EXPR), __FILE__, __LINE__);                            \
  })

/* Macros for counting and rounding. */
#define howmany(x, y)   (((x)+((y)-1))/(y))
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#define rounddown(x,y)  (((x)/(y))*(y))

typedef long ssize_t;

void putchar(int c);
int getchar(void);
void puts(const char *str);
void printf(const char *fmt, ...);
void bzero(void *b, size_t length);
void *memmove(void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
size_t strlen(const char *s);
void *sbrk(size_t size);

#endif // #ifndef KLIBC_H
