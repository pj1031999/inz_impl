/*-
 * Copyright (c) 2004 Olivier Houchard
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

#include "genassym.h"
#include <aarch64/frame.h>
#include <aarch64/ctx.h>

ASSYM(TF_SIZE, sizeof(struct trapframe));
ASSYM(TF_SP, offsetof(struct trapframe, tf_sp));
ASSYM(TF_ELR, offsetof(struct trapframe, tf_elr));
ASSYM(TF_SPSR, offsetof(struct trapframe, tf_spsr)); 
ASSYM(TF_X, offsetof(struct trapframe, tf_x)); 

ASSYM(SF_SIZE, sizeof(struct ctx_t));
ASSYM(SF_SP,   offsetof(struct ctx_t, sf_sp));
ASSYM(SF_SPSR, offsetof(struct ctx_t, sf_spsr)); 
ASSYM(SF_REGS, offsetof(struct ctx_t, sf_regs)); 
ASSYM(SF_LR,   offsetof(struct ctx_t, sf_lr)); 
ASSYM(SF_PC,   offsetof(struct ctx_t, sf_pc)); 
