/****************************************************************************
 * platform/tcc893x/tcc_asm.S
 * Copyright (C) 2014 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andlimitations under the License.
 ****************************************************************************/

#include <asm.h>
#include <arch/ops.h>
#include <arch/defines.h>
#include <platform/sram_map.h>

#undef ENTRY
#undef ENDPROC
#define ENTRY(x)    FUNCTION(x)
#define ENDPROC(x)

/*
  CPSR Mode Bit Definition
*/
#define Mode_USR            0x10
#define Mode_FIQ            0x11
#define Mode_IRQ            0x12
#define Mode_SVC            0x13
#define Mode_ABT            0x17
#define Mode_UND            0x1B
#define Mode_SYS            0x1F
#define Mode_MASK           0x1F
#define NOINT               0xC0
#define I_Bit               0x80
#define F_Bit               0x40

/*
  TRUSTZONE TZASC REGISTERS
*/
#define TZASC_DMEM_R0_ATR   0x60300108
#define TZASC_PERI_R0_ATR   0x60310108
#define TZASC_IMEM_R0_ATR   0x60320108

.text

ENTRY(IO_ARM_SetCPSR)
    msr     cpsr_cf, r0
    bx      lr                  @mov    pc, lr
ENDPROC(IO_ARM_SetCPSR)


ENTRY(IO_ARM_GetCPSR)
    mrs     r0, cpsr
    bx      lr                  @mov    pc, lr
ENDPROC(IO_ARM_GetCPSR)


ENTRY(IO_ARM_ChangeStackSRAM)
    mov     r0, r13
    ldr     r13, =SRAM_STACK_ADDR
    mov     pc, lr
ENDPROC(IO_ARM_ChangeStackSRAM)


ENTRY(IO_ARM_RestoreStackSRAM)
    mov     r13, r0
    mov     pc, lr
ENDPROC(IO_ARM_RestoreStackSRAM)

#ifdef CONFIG_ARM_TRUSTZONE

ENTRY(shutdown_mode)

    //; shut-down func address
    mov    r12, r0

    ldr r0, =CPU_DATA_REPOSITORY_ADDR
    ldr r1, =0x18C818C8
    str r1, [r0]

    ldr sp, =SRAM_STACK_ADDR
    //; Jump to shutdown()
    mov pc, r12
ENDPROC(shutdown_mode)

//;=======================
//; TRUSTZONE
//;=======================
#define TZASC_DMEM_R0_ATR    0x60300108
#define TZASC_PERI_R0_ATR    0x60310108
#define TZASC_IMEM_R0_ATR    0x60320108

//---------------------------------------------------------------------
ENTRY(SRAM_Boot)
//    ldr    pc,    =0xF0000004 //;Jump to remap address for MMU control.

//    ; TZASC set all-regions to non-secure --> uskim @ 2012.08.22
    ldr r0, =TZASC_DMEM_R0_ATR
    mov r1, #0xf0000000
    str r1, [r0]
    add r0,r0,#0x10000
    str r1, [r0]
    add r0,r0,#0x10000
    str r1, [r0]

    ldr r0, =0x60330040
    ldr r1, [r0]
    bic r1, r1, #0x7
    str r1, [r0]

    //check cpuid
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #0x3
    mov r1, #0
    cmp r0, r1
    ldrne pc, =0x30000000

//wakeup_test_loop:
//    b wakeup_test_loop

    //; Mask IRQ/FIQ
    mrs    r0, cpsr
    orr    r0, r0, #0xC0
    msr    cpsr_cf, r0

    //; Set sp(stack point)
    ldr r0, =SRAM_STACK_ADDR
    bic r0, r0, #(0xF<<28)
    mov r13, r0
    bic r13, r13, #(0xF<<28)

    //; Jump to wakeup()
    ldr r0, =WAKEUP_FUNC_ADDR
    bic r0, r0, #(0xF<<28)
    mov lr, pc
    mov pc, r0

    //; Jump to sdram_init()
    ldr r0, =SDRAM_INIT_FUNC_ADDR
    bic r0, r0, #(0xF<<28)
    mov lr, pc
    mov pc, r0

    ldr        r1, =0x82000000
    mov pc, r1
ENDPROC(SRAM_Boot)

#endif
