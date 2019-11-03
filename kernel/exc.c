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

#include <aarch64/aarch64reg.h>
#include <aarch64/cpureg.h>
#include <klibc.h>
#include <pmap.h>
#include <aarch64/frame.h>
#include <aarch64/pte.h>
#include <pcpu.h>

#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

static void print_registers(struct trapframe *frame) {
  unsigned int reg;

  for (reg = 0; reg < nitems(frame->tf_x); reg++) {
    printf(" %sx%d: %16lx\n", (reg < 10) ? " " : "", reg, frame->tf_x[reg]);
  }
  printf("  sp: %16lx\n", frame->tf_sp);
  printf("  lr: %16lx\n", frame->tf_lr);
  printf(" elr: %16lx\n", frame->tf_elr);
  printf("spsr:         %8x\n", frame->tf_spsr);
}

void unhandled_exception(struct trapframe *frame) {
  uint64_t far = reg_far_el1_read();
  uint64_t esr = reg_esr_el1_read();

  print_registers(frame);
  printf(" far: %16lx\n", far);
  printf(" esr:         %.8lx\n", esr);
}

void svc(uint64_t id, struct trapframe *frame) {

  switch (id) {
    case 0x0:
      printf("(exit)\n");
      frame->tf_lr = frame->tf_elr = frame->tf_lr;
      frame->tf_spsr |= SPSR_M_EL1H;
      break;

    case 0x1: // read  number
    {
      int8_t c = 0;
      int64_t num = -1;
      while ((c = getchar()) != '\n') {
        int i = c - '0';
        printf("%c", c);
        if (i < 0 || i > 9) {
          printf("\n");
          frame->tf_x[0] = num;
          break;
        }
        if (num == -1)
          num = 0;
        num = num * 10 + i;
      }
    }

    break;

    case 0x2: // write number
      printf("%d\n", frame->tf_x[0]);
      break;

    default:
      printf("SVC: %d\n", id);
      break;
  }
}

void kpanic() {
  for (;;)
    ;
}

void do_el1h_sync(struct trapframe *frame) {
  uint32_t exception;
  uint64_t esr, dfsc;
  vaddr_t far;

  /* Read the esr register to get the exception details */
  esr = frame->tf_esr; // reg_esr_el1_read();
  exception = ESR_ELx_EXCEPTION(esr);

  switch (exception) {
    case ESR_EC_SVC_A64:
      dfsc = esr & ESR_ISS_HVC_IMM16;
      svc(dfsc, frame);
      return;

    case ESR_EC_DATA_ABT_EL1:
      far = reg_far_el1_read();
      dfsc = esr & ESR_ISS_DATAABORT_DFSC;

      if (dfsc == ESR_ISS_FSC_ACCESS_FAULT_3 ||
          dfsc == ESR_ISS_FSC_ACCESS_FAULT_2) {
        pmap_data_abort_access_fault(far);
        return;
      }
      if (dfsc == ESR_ISS_FSC_PERM_FAULT_3) {
        pmap_data_abort_modify_fault(far);
        // frame->tf_elr = frame->tf_elr-8;
        return;
      }

      if (pcpu()->on_fault) {
        frame->tf_x[0] = false;
        frame->tf_elr = frame->tf_elr + 8;
        return;
      }

      break;

    default:
      break;
  }

  // kpanic();

  unhandled_exception(frame);
  panic(__func__);
}

void do_el0_sync(struct trapframe *frame) {
  uint32_t exception;
  uint64_t esr, dfsc;

  /* Read the esr register to get the exception details */
  esr = reg_esr_el1_read(); // frame->tf_esr;
  exception = ESR_ELx_EXCEPTION(esr);

  switch (exception) {
    case ESR_EC_SVC_A64:
      dfsc = esr & ESR_ISS_HVC_IMM16;
      svc(dfsc, frame);
      return;

    default:
      break;
  }

  // kpanic();

  unhandled_exception(frame);
  panic(__func__);
}

#define bad_trap_panic(trapfunc)                                               \
  void trapfunc(struct trapframe *tf) {                                        \
    printf("%s\n", __func__);                                                  \
    unhandled_exception(tf);                                                   \
    printf("Unhandled:  ");                                                    \
    panic(__func__);                                                           \
  }

bad_trap_panic(trap_el1t_sync) bad_trap_panic(trap_el1t_irq)
  bad_trap_panic(trap_el1t_fiq) bad_trap_panic(trap_el1t_error)

    bad_trap_panic(trap_el1h_fiq) bad_trap_panic(trap_el1h_error)

      bad_trap_panic(trap_el0_fiq) bad_trap_panic(trap_el0_error)

        bad_trap_panic(trap_el0_32sync) bad_trap_panic(trap_el0_32irq)
          bad_trap_panic(trap_el0_32fiq) bad_trap_panic(trap_el0_32error)
