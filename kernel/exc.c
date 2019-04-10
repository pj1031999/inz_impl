/*-
 * Copyright (c) 2014 Andrew Turner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#include <aarch64/cpureg.h>
#include <klibc.h>
#include <aarch64/frame.h>

#define nitems(x)   (sizeof((x)) / sizeof((x)[0]))

static void print_registers(struct trapframe *frame)
{
  unsigned int reg;

  for (reg = 0; reg < nitems(frame->tf_x); reg++) {
    printf(" %sx%d: %16lx\n", (reg < 10) ? " " : "", reg,
  	   frame->tf_x[reg]);
  }
  printf("  sp: %16lx\n", frame->tf_sp);
  printf("  lr: %16lx\n", frame->tf_lr);
  printf(" elr: %16lx\n", frame->tf_elr);
  printf("spsr:         %8x\n", frame->tf_spsr);
}


/* #define FAULT_WRITE	0x0800	/\* fault was due to write (ARMv6+) *\/ */
/* #define FAULT_TYPE_MASK 0x0f */
/*   printf("Illegal %s access at %p (type=%x)!\n", */
/*          (dfsr & FAULT_WRITE) ? "write" : "read", dfar, dfsr & FAULT_TYPE_MASK); */

void unhandled_exception(struct trapframe *frame);

void do_el1h_sync(__unused struct trapframe *frame)
{
  unhandled_exception(frame);
}

void do_el0_sync(__unused struct trapframe *frame)
{
  unhandled_exception(frame);
}

void do_serror(struct trapframe *frame)
{
  uint64_t esr, far;

  far = reg_far_el1_read();
  esr = reg_esr_el1_read();
  
  print_registers(frame);
  printf(" far: %16lx\n", far);
  printf(" esr:         %.8lx\n", esr);
  panic("Unhandled System Error");
}

void unhandled_exception(struct trapframe *frame)
{
  uint64_t esr, far;

  far = reg_far_el1_read();
  esr = reg_esr_el1_read();
  
  print_registers(frame);
  printf(" far: %16lx\n", far);
  printf(" esr:         %.8lx\n", esr);
  panic("Unhandled exception");
}

#define bad_trap_panic(trapfunc)		\
  void trapfunc(struct trapframe *tf)		\
  {						\
    printf("%s\n", __func__);			\
    unhandled_exception(tf);			\
  }
bad_trap_panic(trap_el1t_sync)
bad_trap_panic(trap_el1t_irq)
bad_trap_panic(trap_el1t_fiq)
bad_trap_panic(trap_el1t_error)
bad_trap_panic(trap_el1h_fiq)
bad_trap_panic(trap_el1h_error)
bad_trap_panic(trap_el0_fiq)
bad_trap_panic(trap_el0_error)
bad_trap_panic(trap_el0_32fiq)
bad_trap_panic(trap_el0_32error)
