#ifndef DEMO_H
#define DEMO_H

#include <klibc.h>
#include <pmman.h>
#include <pmap.h>
#include <pcpu.h>

void demo_clock();
void demo_pmap();
void demo_uart();
void demo_led();
void demo_gpio();

extern vaddr_t _brk_limit;

vaddr_t vm_alloc(size_t size);
vaddr_t pages_alloc(size_t pages, flags_t flags);


#endif /*DEMO_H*/
