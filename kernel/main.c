/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Copyright (c) 2009-2013, The Linux Foundation. All rights reserved.
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
#include <compiler.h>
#include <debug.h>
#include <string.h>
#include <app.h>
#include <arch.h>
#include <platform.h>
#include <target.h>
#include <lib/heap.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <kernel/dpc.h>
#include <boot_stats.h>

#if defined(BOOTING_BY_REMOTE_KEY) || defined(RECOVERY_BY_REMOTE_KEY) || defined(START_FWDN_BY_REMOTE_KEY)
#include <tcc_remocon.h>
#endif

extern void *__ctor_list;
extern void *__ctor_end;
extern int __bss_start;
extern int _end;

static int bootstrap2(void *arg);

#if (ENABLE_NANDWRITE)
void bootstrap_nandwrite(void);
#endif

static void call_constructors(void)
{
	void **ctor;

	ctor = &__ctor_list;
	while(ctor != &__ctor_end) {
		void (*func)(void);

		func = (void (*)())*ctor;

		func();
		ctor++;
	}
}

#ifdef CONFIG_ARM_TRUSTZONE
int lk_in_normalworld = 0;  // lk runs on SW when FWDN mode.
void _get_ns(void)
{
	unsigned int cpsr = 0, nsacr = 0;
	__asm__ volatile(
			"mrc	p15, 0, %[nsacr], cr1, cr1, 2\n"
			"mrs %[cpsr], cpsr\n" :
			[nsacr]"=r" (nsacr),
			[cpsr]"=r"(cpsr));
	if (nsacr == 0 || ((cpsr & 0x1F) == 0b10110))
		lk_in_normalworld = 0;
	else
		lk_in_normalworld = 1;
}

#define TZOS_MAGIC_1	0x21458E6A
#define TZOS_MAGIC_2	0x94C6289B
#define TZOS_MAGIC_3	0xFA89ED03
#define TZOS_MAGIC_4	0x9968728F
#define TZOS_MAGIC_M	0xA372B85C

unsigned int tzos_magic[4] =
	{TZOS_MAGIC_1, TZOS_MAGIC_2, TZOS_MAGIC_3, TZOS_MAGIC_4};

extern void _secure_vector(void);
unsigned int tzos_load(void)
{
	if((tzos_magic[0] == TZOS_MAGIC_M)&&(check_fwdn_mode()==0))
	{
		memcpy((void *)tzos_magic[1], (void *)(MEMBASE+tzos_magic[2]), tzos_magic[3]);
		return tzos_magic[1];
	}
	else
		return 0;
}
#endif

/* called from crt0.S */
void kmain(void) __NO_RETURN __EXTERNALLY_VISIBLE;
void kmain(void)
{
#ifdef CONFIG_ARM_TRUSTZONE
	_get_ns();
#endif

	// get us into some sort of thread context
	thread_init_early();

	// early arch stuff
	arch_early_init();

	// do any super early platform initialization
	platform_early_init();

	// do any super early target initialization
	target_early_init();

#ifdef CONFIG_ARM_TRUSTZONE
	dprintf(INFO, "welcome to lk (ns = %d)!\n\n", lk_in_normalworld);
#else
	dprintf(INFO, "welcome to lk\n\n");
#endif
	bs_set_timestamp(BS_BL_START);

	// deal with any static constructors
	dprintf(SPEW, "calling constructors\n");
	call_constructors();

	// bring up the kernel heap
	dprintf(SPEW, "initializing heap\n");
	heap_init();

	// initialize the threading system
	dprintf(SPEW, "initializing threads\n");
	thread_init();

	// initialize the dpc system
	dprintf(SPEW, "initializing dpc\n");
	dpc_init();

	// initialize kernel timers
	dprintf(SPEW, "initializing timers\n");
	timer_init();

#if defined(BOOTING_BY_REMOTE_KEY) || defined(RECOVERY_BY_REMOTE_KEY) || defined(START_FWDN_BY_REMOTE_KEY)
	if (!check_fwdn_mode() && check_reboot_mode() != 0x77665502) {
		initRemocon();
		#if defined(BOOTING_BY_REMOTE_KEY)
		dprintf(INFO, "Wait a power key..\n\n");
		mdelay(1);
		do {
			setUseRemoteIrq(0);
		} while (getRemoteKey() != SCAN_POWER);
		#endif
		#if defined(RECOVERY_BY_REMOTE_KEY) || defined(START_FWDN_BY_REMOTE_KEY)
		setUseRemoteIrq(1);
		#endif
	}
#endif

#if (!ENABLE_NANDWRITE)
	// create a thread to complete system initialization
	dprintf(SPEW, "creating bootstrap completion thread\n");
	thread_resume(thread_create("bootstrap2", &bootstrap2, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));

	// enable interrupts
	exit_critical_section();

	// become the idle thread
	thread_become_idle();
#else
        bootstrap_nandwrite();
#endif
}

int main(void);

static int bootstrap2(void *arg)
{
	dprintf(SPEW, "top of bootstrap2()\n");

	arch_init();

	// XXX put this somewhere else
#if WITH_LIB_BIO
	bio_init();
#endif
#if WITH_LIB_FS
	fs_init();
#endif

	// initialize the rest of the platform
	dprintf(SPEW, "initializing platform\n");
	platform_init();

	// initialize the target
	dprintf(SPEW, "initializing target\n");
	target_init();

	dprintf(SPEW, "calling apps_init()\n");
	apps_init();

	return 0;
}

#if (ENABLE_NANDWRITE)
void bootstrap_nandwrite(void)
{
	dprintf(SPEW, "top of bootstrap2()\n");

	arch_init();

	// initialize the rest of the platform
	dprintf(SPEW, "initializing platform\n");
	platform_init();

	// initialize the target
	dprintf(SPEW, "initializing target\n");
	target_init();

	dprintf(SPEW, "calling nandwrite_init()\n");
	nandwrite_init();

	return 0;
}
#endif

#ifdef OTP_UID_INCLUDE
#include "otp_uid_include.h"
void copy_secure_bootloader(void)
{
	memcpy((void*)TARGET_ADDR, (void*)(MEMBASE+NORMAL_LK_SIZE), SECURE_LK_SIZE);
}
#endif
