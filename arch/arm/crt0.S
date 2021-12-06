/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define DSB .byte 0x4f, 0xf0, 0x7f, 0xf5
#define ISB .byte 0x6f, 0xf0, 0x7f, 0xf5

.section ".text.boot"
.globl _start
_start:
#if PLATFORM_TCC
	b	_vectors
/*
 *************************************************************************
 * Telechips Board data
 *************************************************************************
 */ 
	.word 0xffff0106	/* HardwareID */
.globl FirmwareVersion
FirmwareVersion:
	.word 0x3A726556	/* Ver: */
	.word 0x34333231	/* 1234 */
FirmwareCheckSum:
	.word 0x00000000	/* Firmware CRC32 Value Until 128Kbyte */
DACVersion:
	.word 0x00000000	/* Not Used Area - Reserved */
FirmwareCheckSumEnd:
	.word 0x00000000	/* Firmware CRC32 Value From 128Kbye to End */
FirmwareSize:
	.word 0x00000000	/* Firmware Total Size */
SerialNumber:
	.word 0x00000000	/* SN[3:0]   */
	.word 0x00000000	/* SN[7:4]   */
	.word 0x00000000	/* SN[11:8]  */
	.word 0x00000000	/* SN[15:12] */
	.word 0x00000000	/* SN[19:16] */
	.word 0x00000000	/* SN[23:20] */
	.word 0x00000000	/* SN[27:24] */
	.word 0x00000000	/* SN[31:28] */
	.word 0x00000000	/* SN[35:32] */
	.word 0x00000000	/* SN[39:36] */
	.word 0x00000000	/* SN[43:40] */
	.word 0x00000000	/* SN[47:44] */
	.word 0x00000000	/* SN[51:48] */
	.word 0x00000000	/* SN[55:52] */
	.word 0x00000000	/* SN[59:56] */
	.word 0x00000000	/* SN[63:60] */
.globl FirmwareBaseAddress
FirmwareBaseAddress:
	.word _start		/* base address */
	.word 0x00000000	/* reserved for tcc */
#ifdef FWDN_V7
	.word 0x20203756	/* "V7	" */
#else
	.word 0x544f4f42	/* "BOOT" */
#endif

#if defined(TCC893X)
	.word 0x58333938	/* "893X" */
#elif defined(TCC896X)
	.word 0x58363938	/* "896X" */
#elif defined(TCC897X)
	.word 0x58373938	/* "897X" */
#else
	#err Should be select CHIPSET
#endif

.globl InitRoutine_StartBase
InitRoutine_StartBase:
	.word InitRoutine_Start
.globl InitRoutine_StartLimit
InitRoutine_StartLimit:
	.word InitRoutine_End

//=========================================================================
//NAND Boot & Update Hidden Area Information
//-------------------------------------------------------------------------
#if _EMMC_BOOT
#else
		.word TNFTL_STRUCTURE_VERSION
		.word BL1_NAND_ROM
#endif

.globl BOOT_AREA_SIZE_IN_MB
BOOT_AREA_SIZE_IN_MB:
		.word	2				// 2MB

.globl RO_0_AREA_SECTOR_COUNT
RO_0_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl RO_1_AREA_SECTOR_COUNT
RO_1_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl RO_2_AREA_SECTOR_COUNT
RO_2_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl RO_3_AREA_SECTOR_COUNT
RO_3_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl RW_0_AREA_SECTOR_COUNT
RW_0_AREA_SECTOR_COUNT:
#if defined(SECURE_KEYBOX_INCLUDE)
		.word 0x00002000
#else
#if defined(GPH_AREA_INCLUDE)||defined(SECURE_UID_INCLUDE)
		.word 0x00000800
#else
		.word 0x00000000
#endif
#endif

.globl RW_1_AREA_SECTOR_COUNT
RW_1_AREA_SECTOR_COUNT:
#if defined(SECURE_KEYBOX_INCLUDE)
		.word 0x00008000
#else
#if defined(GPH_AREA_INCLUDE)||defined(SECURE_UID_INCLUDE)
		.word 0x00000800
#else
		.word 0x00000000
#endif
#endif

.globl RW_2_AREA_SECTOR_COUNT
RW_2_AREA_SECTOR_COUNT:
#if defined(SECURE_UID_INCLUDE)
		.word 0x00002000
#else
#if defined(GPH_AREA_INCLUDE)
		.word 0x00004000
#else
		.word 0x00000000
#endif
#endif

.globl RW_3_AREA_SECTOR_COUNT
RW_3_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl RW_4_AREA_SECTOR_COUNT
RW_4_AREA_SECTOR_COUNT:
		.word 0x00000000

.globl DATA_AREA_SECTOR_COUNT
DATA_AREA_SECTOR_COUNT:
		.word 0xFFFFFFFF

//	ENDIF
//=========================================================================
//-------------------------------------------------------------------------

.globl _vectors
_vectors:
	b		reset
	ldr		pc, _arm_undefined
	ldr		pc, _arm_syscall
	ldr		pc, _arm_prefetch_abort
	ldr		pc, _arm_data_abort
	ldr		pc, _arm_reserved
	ldr		pc, _arm_irq
	ldr		pc, _arm_fiq

_arm_undefined:
	.word		arm_undefined
_arm_syscall:
	.word		arm_syscall
_arm_prefetch_abort:
	.word		arm_prefetch_abort
_arm_data_abort:
	.word		arm_data_abort
_arm_reserved:
	.word		arm_reserved
_arm_irq:
	.word		arm_irq
_arm_fiq:
	.word		arm_fiq
#else
	b	reset
	b	arm_undefined
	b	arm_syscall
	b	arm_prefetch_abort
	b	arm_data_abort
	b	arm_reserved
	b	arm_irq
	b	arm_fiq
#endif

reset:
#if !defined(TCC893X)
	bl	Init_SoCs
#endif

#ifdef CONFIG_ARM_TRUSTZONE
#if defined(TCC893X)
	bl	early_shutdown_enter
	ldr	r2, =0x10010000  /* SRAM phys. address */
#elif defined(TCC896X)
	ldr	r2, =0xF0010000  /* SRAM phys. address */
#else
	ldr	r2, =0x10010000  /* SRAM phys. address */
#endif
	mov	sp, r2

	bl	tzos_load
	cmp	r0, #0
	beq	tzos_skip
	mov	r1, r0
	ldr	r0, =0x0
	mov	lr, pc
	mov	pc, r1
tzos_skip:
#endif

#ifdef OTP_UID_INCLUDE
	bl	copy_secure_bootloader
#endif

	/* do some cpu setup */
#if ARM_WITH_CP15
        /* Read SCTLR */
	mrc		p15, 0, r0, c1, c0, 0
		/* XXX this is currently for arm926, revist with armv6 cores */
		/* new thumb behavior, low exception vectors, i/d cache disable, mmu disabled */
	bic		r0, r0, #(1<<15| 1<<13 | 1<<12)
	bic		r0, r0, #(1<<2 | 1<<0)
		/* enable alignment faults */
	orr		r0, r0, #(1<<1)
        /* Write SCTLR */
	mcr		p15, 0, r0, c1, c0, 0

#ifdef ENABLE_TRUSTZONE
  /*nkazi: not needed ? Setting VBAR to location of new vector table : 0x80000      */
 ldr             r0, =0x00080000
 mcr             p15, 0, r0, c12, c0, 0
#endif

#endif

#if WITH_CPU_EARLY_INIT
	/* call platform/arch/etc specific init code */
	bl __cpu_early_init

	/* declare return address as global to avoid using stack */
.globl _cpu_early_init_complete
	_cpu_early_init_complete:

#endif

#if (!ENABLE_NANDWRITE)
#if WITH_CPU_WARM_BOOT
	ldr 	r0, warm_boot_tag
	cmp 	r0, #1

	/* if set, warm boot */
	ldreq 	pc, =BASE_ADDR

	mov 	r0, #1
	str	r0, warm_boot_tag
#endif
#endif

	/* see if we need to relocate */
	mov		r0, pc
	sub		r0, r0, #(.Laddr - _start)
.Laddr:
	ldr		r1, =_start
	cmp		r0, r1
	beq		.Lstack_setup

	/* we need to relocate ourselves to the proper spot */
	ldr		r2, =__data_end	

.Lrelocate_loop:
	ldr		r3, [r0], #4
	str		r3, [r1], #4
	cmp		r1, r2
	bne		.Lrelocate_loop

	/* we're relocated, jump to the right address */
	ldr		r0, =.Lstack_setup
	bx		r0

.ltorg
#if WITH_CPU_WARM_BOOT
warm_boot_tag:
	.word 0
#endif

.Lstack_setup:
	/* set up the stack for irq, fiq, abort, undefined, system/user, and lastly supervisor mode */
	mrs     r0, cpsr
	bic     r0, r0, #0x1f

	ldr		r2, =abort_stack_top
	orr     r1, r0, #0x12 // irq
	msr     cpsr_c, r1
	ldr		r13, =irq_save_spot		/* save a pointer to a temporary dumping spot used during irq delivery */
	    
	orr     r1, r0, #0x11 // fiq
	msr     cpsr_c, r1
	mov		sp, r2
	            
	orr     r1, r0, #0x17 // abort
	msr     cpsr_c, r1
	mov		sp, r2
	    
	orr     r1, r0, #0x1b // undefined
	msr     cpsr_c, r1
	mov		sp, r2
	    
	orr     r1, r0, #0x1f // system
	msr     cpsr_c, r1
	mov		sp, r2

	orr		r1, r0, #0x13 // supervisor
	msr		cpsr_c, r1
	mov		sp, r2

	/* copy the initialized data segment out of rom if necessary */
	ldr		r0, =__data_start_rom
	ldr		r1, =__data_start
	ldr		r2, =__data_end

	cmp		r0, r1
	beq		.L__do_bss

.L__copy_loop:
	cmp		r1, r2
	ldrlt	r3, [r0], #4
	strlt	r3, [r1], #4
	blt		.L__copy_loop

.L__do_bss:
    /* clear out the bss */
    ldr     r0, =__bss_start
    ldr     r2, =_end
    mov     r1, #0
    sub     r2, r2, r0
    bl      memset

#if 0
.L__do_bss:
	/* clear out the bss */
	ldr		r0, =__bss_start
	ldr		r1, =_end
	mov		r2, #0
.L__bss_loop:
	cmp		r0, r1
	strlt	r2, [r0], #4
	blt		.L__bss_loop
#endif

#ifdef ARM_ISA_ARMV7
	DSB
	ISB
#endif

	bl		kmain
	b		.

.ltorg

.bss
.align 2
	/* the abort stack is for unrecoverable errors.
	 * also note the initial working stack is set to here.
	 * when the threading system starts up it'll switch to a new 
	 * dynamically allocated stack, so we don't need it for very long
	 */
abort_stack:
	.skip 1024
abort_stack_top:
