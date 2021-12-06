/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 * Copyright (c) 2009-2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *	may be used to endorse or promote products derived from this
 *	software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <dev/keys.h>
#include <dev/gpio_keypad.h>
#include <plat/cpu.h>

/* don't turn this on without updating the ffa support */
#define SCAN_FUNCTION_KEYS 0

#include <platform/gpio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

static unsigned int output_gpios[] = {
};

#define KEYMAP_INDEX(out, in) ((out)*ARRAY_SIZE(output_gpios) + (in))

static const unsigned short tcc_keymap[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 2)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(0, 3)] = KEY_HOME,
	[KEYMAP_INDEX(0, 4)] = KEY_BACK,
};

static unsigned int input_gpios[] = {
	TCC_GPE(26), TCC_GPE(27), TCC_GPE(28), TCC_GPE(29), TCC_GPE(30)
};

static struct gpio_keypad_info tcc_keypad_info = {
	.keymap		= tcc_keymap,
	.output_gpios	= output_gpios,
	.input_gpios	= input_gpios,
	.noutputs	= ARRAY_SIZE(output_gpios),
	.ninputs	= ARRAY_SIZE(input_gpios),
	.settle_time	= 5 /* msec */,
	.poll_time	= 20 /* msec */,
	.flags		= GPIOKPF_DRIVE_INACTIVE,
};

void keypad_init(void)
{
#ifndef NO_GPIO_KEYPAD
	gpio_keypad_init(&tcc_keypad_info);
#endif
}
