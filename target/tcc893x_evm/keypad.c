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

/* don't turn this on without updating the ffa support */
#define SCAN_FUNCTION_KEYS 0

#include <platform/gpio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#if (HW_REV == 0x1000)
static unsigned int tcc_col_gpios[] = {
	TCC_GPB(24), TCC_GPB(25), TCC_GPB(26)
};

static unsigned int tcc_row_gpios[] = {
	TCC_GPB(27), TCC_GPB(29)
};
#elif (HW_REV == 0x2000 || HW_REV == 0x3000)
static unsigned int tcc_col_gpios[] = {
	TCC_GPE(15), TCC_GPE(16), TCC_GPE(21)
};

static unsigned int tcc_row_gpios[] = {
	TCC_GPE(22), TCC_GPE(23)
};
#endif

#define KEYMAP_INDEX(row, col) ((row)*ARRAY_SIZE(tcc_col_gpios) + (col))

static const unsigned short tcc_keymap[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX(0, 1)] = KEY_F1,
	[KEYMAP_INDEX(0, 2)] = KEY_VOLUMEUP,

	[KEYMAP_INDEX(1, 0)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(1, 1)] = KEY_HOME,
	[KEYMAP_INDEX(1, 2)] = KEY_BACK,
};

static struct gpio_keypad_info tcc_keypad_info = {
	.keymap		= tcc_keymap,
	.output_gpios	= tcc_col_gpios,
	.input_gpios	= tcc_row_gpios,
	.noutputs	= ARRAY_SIZE(tcc_col_gpios),
	.ninputs	= ARRAY_SIZE(tcc_row_gpios),
	.settle_time	= 5 /* msec */,
	.poll_time	= 20 /* msec */,
	.flags		= GPIOKPF_DRIVE_INACTIVE,
};

void keypad_init(void)
{
	gpio_keypad_init(&tcc_keypad_info);
}
