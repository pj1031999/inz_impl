#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>
#include <stddef.h>

void *kmem_sbrk(size_t size);

#endif // #ifndef KMEM_H
