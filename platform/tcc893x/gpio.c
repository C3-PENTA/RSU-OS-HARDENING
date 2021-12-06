/*
 * Copyright (C) 2010 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <debug.h>
#include <reg.h>
#include <dev/gpio.h>
#include <platform/iomap.h>
#include <platform/gpio.h>

typedef struct gpioregs gpioregs;

#define EXT_GPIO_MAX	4
static struct ext_gpio *ext_gpio[EXT_GPIO_MAX] = { NULL, };

#define GPIO_REG(off)	(TCC_GPIO_BASE + (off))

struct gpioregs
{
	unsigned data;         /* data */
	unsigned out_en;       /* output enable */
	unsigned out_or;       /* OR fnction on output data */
	unsigned out_bic;      /* BIC function on output data */
	unsigned out_xor;      /* XOR function on output data */
	unsigned strength0;    /* driver strength control 0 */
	unsigned strength1;    /* driver strength control 1 */
	unsigned pull_enable;  /* pull-up/down enable */
	unsigned pull_select;  /* pull-up/down select */
	unsigned in_en;        /* input enable */
	unsigned in_type;      /* input type (Shmitt / CMOS) */
	unsigned slew_rate;    /* slew rate */
	unsigned func_select0; /* port configuration 0 */
	unsigned func_select1; /* port configuration 1 */
	unsigned func_select2; /* port configuration 2 */
	unsigned func_select3; /* port configuration 3 */
};

static gpioregs GPIO_REGS[] = {
	{ /* GPIO A */
		.data         = GPIO_REG(0x000),
		.out_en       = GPIO_REG(0x004),
		.out_or       = GPIO_REG(0x008),
		.out_bic      = GPIO_REG(0x00C),
		.out_xor      = GPIO_REG(0x010),
		.strength0    = GPIO_REG(0x014),
		.strength1    = GPIO_REG(0x018),
		.pull_enable  = GPIO_REG(0x01C),
		.pull_select  = GPIO_REG(0x020),
		.in_en        = GPIO_REG(0x024),
		.in_type      = GPIO_REG(0x028),
		.slew_rate    = GPIO_REG(0x02C),
		.func_select0 = GPIO_REG(0x030),
		.func_select1 = GPIO_REG(0x034),
		.func_select2 = GPIO_REG(0x038),
		.func_select3 = GPIO_REG(0x03C),
	},
	{ /* GPIO B */
		.data         = GPIO_REG(0x040),
		.out_en       = GPIO_REG(0x044),
		.out_or       = GPIO_REG(0x048),
		.out_bic      = GPIO_REG(0x04C),
		.out_xor      = GPIO_REG(0x050),
		.strength0    = GPIO_REG(0x054),
		.strength1    = GPIO_REG(0x058),
		.pull_enable  = GPIO_REG(0x05C),
		.pull_select  = GPIO_REG(0x060),
		.in_en        = GPIO_REG(0x064),
		.in_type      = GPIO_REG(0x068),
		.slew_rate    = GPIO_REG(0x06C),
		.func_select0 = GPIO_REG(0x070),
		.func_select1 = GPIO_REG(0x074),
		.func_select2 = GPIO_REG(0x078),
		.func_select3 = GPIO_REG(0x07C),
	},
	{ /* GPIO C */
		.data         = GPIO_REG(0x080),
		.out_en       = GPIO_REG(0x084),
		.out_or       = GPIO_REG(0x088),
		.out_bic      = GPIO_REG(0x08C),
		.out_xor      = GPIO_REG(0x090),
		.strength0    = GPIO_REG(0x094),
		.strength1    = GPIO_REG(0x098),
		.pull_enable  = GPIO_REG(0x09C),
		.pull_select  = GPIO_REG(0x0A0),
		.in_en        = GPIO_REG(0x0A4),
		.in_type      = GPIO_REG(0x0A8),
		.slew_rate    = GPIO_REG(0x0AC),
		.func_select0 = GPIO_REG(0x0B0),
		.func_select1 = GPIO_REG(0x0B4),
		.func_select2 = GPIO_REG(0x0B8),
		.func_select3 = GPIO_REG(0x0BC),
	},
	{ /* GPIO D */
		.data         = GPIO_REG(0x0C0),
		.out_en       = GPIO_REG(0x0C4),
		.out_or       = GPIO_REG(0x0C8),
		.out_bic      = GPIO_REG(0x0CC),
		.out_xor      = GPIO_REG(0x0D0),
		.strength0    = GPIO_REG(0x0D4),
		.strength1    = GPIO_REG(0x0D8),
		.pull_enable  = GPIO_REG(0x0DC),
		.pull_select  = GPIO_REG(0x0E0),
		.in_en        = GPIO_REG(0x0E4),
		.in_type      = GPIO_REG(0x0E8),
		.slew_rate    = GPIO_REG(0x0EC),
		.func_select0 = GPIO_REG(0x0F0),
		.func_select1 = GPIO_REG(0x0F4),
		.func_select2 = GPIO_REG(0x0F8),
		.func_select3 = GPIO_REG(0x0FC),
	},
	{ /* GPIO E */
		.data         = GPIO_REG(0x100),
		.out_en       = GPIO_REG(0x104),
		.out_or       = GPIO_REG(0x108),
		.out_bic      = GPIO_REG(0x10C),
		.out_xor      = GPIO_REG(0x110),
		.strength0    = GPIO_REG(0x114),
		.strength1    = GPIO_REG(0x118),
		.pull_enable  = GPIO_REG(0x11C),
		.pull_select  = GPIO_REG(0x120),
		.in_en        = GPIO_REG(0x124),
		.in_type      = GPIO_REG(0x128),
		.slew_rate    = GPIO_REG(0x12C),
		.func_select0 = GPIO_REG(0x130),
		.func_select1 = GPIO_REG(0x134),
		.func_select2 = GPIO_REG(0x138),
		.func_select3 = GPIO_REG(0x13C),
	},
	{ /* GPIO F */
		.data         = GPIO_REG(0x140),
		.out_en       = GPIO_REG(0x144),
		.out_or       = GPIO_REG(0x148),
		.out_bic      = GPIO_REG(0x14C),
		.out_xor      = GPIO_REG(0x150),
		.strength0    = GPIO_REG(0x154),
		.strength1    = GPIO_REG(0x158),
		.pull_enable  = GPIO_REG(0x15C),
		.pull_select  = GPIO_REG(0x160),
		.in_en        = GPIO_REG(0x164),
		.in_type      = GPIO_REG(0x168),
		.slew_rate    = GPIO_REG(0x16C),
		.func_select0 = GPIO_REG(0x170),
		.func_select1 = GPIO_REG(0x174),
		.func_select2 = GPIO_REG(0x178),
		.func_select3 = GPIO_REG(0x17C),
	},
	{ /* GPIO G */
		.data         = GPIO_REG(0x180),
		.out_en       = GPIO_REG(0x184),
		.out_or       = GPIO_REG(0x188),
		.out_bic      = GPIO_REG(0x18C),
		.out_xor      = GPIO_REG(0x190),
		.strength0    = GPIO_REG(0x194),
		.strength1    = GPIO_REG(0x198),
		.pull_enable  = GPIO_REG(0x19C),
		.pull_select  = GPIO_REG(0x1A0),
		.in_en        = GPIO_REG(0x1A4),
		.in_type      = GPIO_REG(0x1A8),
		.slew_rate    = GPIO_REG(0x1AC),
		.func_select0 = GPIO_REG(0x1B0),
		.func_select1 = GPIO_REG(0x1B4),
		.func_select2 = GPIO_REG(0x1B8),
		.func_select3 = GPIO_REG(0x1BC),
	},
	{ /* GPIO HDMI */
		.data         = GPIO_REG(0x1C0),
		.out_en       = GPIO_REG(0x1C4),
		.out_or       = GPIO_REG(0x1C8),
		.out_bic      = GPIO_REG(0x1CC),
		.out_xor      = GPIO_REG(0x1D0),
		.strength0    = GPIO_REG(0x1D4),
		.strength1    = GPIO_REG(0x1D8), //reserved
		.pull_enable  = GPIO_REG(0x1DC),
		.pull_select  = GPIO_REG(0x1E0),
		.in_en        = GPIO_REG(0x1E4),
		.in_type      = GPIO_REG(0x1E8),
		.slew_rate    = GPIO_REG(0x1EC),
		.func_select0 = GPIO_REG(0x1F0),
		.func_select1 = GPIO_REG(0x1F4), //reserved
		.func_select2 = GPIO_REG(0x1F8), //reserved
		.func_select3 = GPIO_REG(0x1FC), //reserved
	},
	{ /* GPIO ADC */
		.data         = GPIO_REG(0x200),
		.out_en       = GPIO_REG(0x204),
		.out_or       = GPIO_REG(0x208),
		.out_bic      = GPIO_REG(0x20C),
		.out_xor      = GPIO_REG(0x210),
		.strength0    = GPIO_REG(0x214),
		.strength1    = GPIO_REG(0x218), //reserved
		.pull_enable  = GPIO_REG(0x21C),
		.pull_select  = GPIO_REG(0x220),
		.in_en        = GPIO_REG(0x224),
		.in_type      = GPIO_REG(0x228),
		.slew_rate    = GPIO_REG(0x22C),
		.func_select0 = GPIO_REG(0x230),
		.func_select1 = GPIO_REG(0x234), //reserved
		.func_select2 = GPIO_REG(0x238), //reserved
		.func_select3 = GPIO_REG(0x23C), //reserved
	},
};

int gpio_config(unsigned n, unsigned flags)
{
	gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	unsigned num = n&GPIO_BITMASK;;
	unsigned bit = (1<<num);

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		unsigned id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		return ext_gpio[id]->config(ext_gpio[id], n, flags);
	}

	if (r == 0) return -1;

	if (flags & GPIO_FN_BITMASK) {
		unsigned fn = ((flags & GPIO_FN_BITMASK) >> GPIO_FN_SHIFT) - 1;

		if (num < 8)
			RMWREG32(r->func_select0, num*4, 4, fn);
		else if (num < 16)
			RMWREG32(r->func_select1, (num-8)*4, 4, fn);
		else if (num < 24)
			RMWREG32(r->func_select2, (num-16)*4, 4, fn);
		else
			RMWREG32(r->func_select3, (num-24)*4, 4, fn);
	}

	if (flags & GPIO_CD_BITMASK) {
		unsigned cd = ((flags & GPIO_CD_BITMASK) >> GPIO_CD_SHIFT) - 1;

		if (num < 16)
			RMWREG32(r->strength0, num*2, 2, cd);
		else
			RMWREG32(r->strength1, (num-16)*2, 2, cd);
	}

	if (flags & GPIO_OUTPUT) {
		if (flags & GPIO_HIGH)
			writel(readl(r->data) | bit, r->data);
		if (flags & GPIO_LOW)
			writel(readl(r->data) & (~bit), r->data);
		writel(readl(r->out_en) | bit, r->out_en);
	} else {
		writel(readl(r->out_en) & (~bit), r->out_en);
	}

	if (flags & GPIO_PULLUP){
		writel(readl(r->pull_select) | bit, r->pull_select);
		writel(readl(r->pull_enable) | bit, r->pull_enable);
	} else if (flags & GPIO_PULLDOWN) {
		writel(readl(r->pull_select) & (~bit), r->pull_select);
		writel(readl(r->pull_enable) | bit, r->pull_enable);
	} else if (flags & GPIO_PULLDISABLE) {
		writel(readl(r->pull_enable) & (~bit), r->pull_enable);
	}

	return 0;
}

void gpio_set(unsigned n, unsigned on)
{
	gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	unsigned num = n&GPIO_BITMASK;;
	unsigned bit = (1<<num);

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		unsigned id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		ext_gpio[id]->set(ext_gpio[id], n, on);
		return;
	}

	if (r != 0) {
		if (on) {
			writel(readl(r->data) | bit, r->data);
		} else {
			writel(readl(r->data) & (~bit), r->data);
		}
	}
}

int gpio_get(unsigned n)
{
	gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	unsigned num = n&GPIO_BITMASK;;
	unsigned bit = (1<<num);

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		unsigned id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		return ext_gpio[id]->get(ext_gpio[id], n);
	}

	if (r == 0) return 0;

	return (readl(r->data) & bit) ? 1 : 0;
}

int register_ext_gpio(unsigned gpio_id, struct ext_gpio *gpios)
{
	unsigned id = ((gpio_id&GPIO_REGMASK)-GPIO_PORTEXT1)>>GPIO_REG_SHIFT;

	if (id >= EXT_GPIO_MAX)
		return -1;

	if (ext_gpio[id])
		return -2;

	ext_gpio[id] = gpios;
	return 0;
}

