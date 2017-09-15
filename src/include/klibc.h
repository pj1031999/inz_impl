#ifndef KLIBC_H
#define KLIBC_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

typedef long ssize_t;

void kprintf(const char *fmt, ...);
void *memchr(const void *s, int c, size_t n);
size_t strlen(const char *s);

#endif // #ifndef KLIBC_H
