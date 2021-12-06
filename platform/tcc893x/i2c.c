/*
 * Copyright (c) 2011 Telechips, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
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


#include <debug.h>
#include <err.h>
#include <reg.h>
#include <string.h>
#include <platform.h>
#include <i2c.h>
#include <platform/iomap.h>
#include <platform/reg_physical.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <platform/tcc_ckc.h>
#include <clock.h>


/* 
 * Define
 */

#ifndef BITCSET
#define BITCSET(X, CMASK, SMASK)	( (X) = ((((unsigned int)(X)) & ~((unsigned int)(CMASK))) | ((unsigned int)(SMASK))) )
#endif

#define read_reg(a)			(*(volatile unsigned long *)a)
#define write_reg(v, a)		(*(volatile unsigned long *)a = v)

/* read/write bit */
#define I2C_WR	0
#define I2C_RD	1

/* 
 * I2C register structure
 */
struct tcc_i2c_regs {
	volatile unsigned long PRES, CTRL, TXR, CMD, RXR, SR, TR;
};

/* 
 * register structure
 */
struct tcc_i2c {
	unsigned int clk;
	unsigned long IRQSTR;
	volatile struct tcc_i2c_regs *regs;
	unsigned int peri_name;
	unsigned int iobus_name;
	int is_reset;
};

static struct tcc_i2c i2c[I2C_CH_NUM] = {
	[I2C_CH_MASTER0] = {
		.clk = 400,
		.IRQSTR = HwI2C_PORTCFG_BASE + 0x0C,
		.regs = (volatile struct tcc_i2c_regs *)HwI2C_MASTER0_BASE,
		.peri_name = PERI_I2C0,
		.iobus_name = IOBUS_I2C_M0,
		.is_reset = 0,
	},
	[I2C_CH_MASTER1] = {
		.clk = 400,
		.IRQSTR = HwI2C_PORTCFG_BASE + 0x0C,
		.regs = (volatile struct tcc_i2c_regs *)HwI2C_MASTER1_BASE,
		.peri_name = PERI_I2C1,
		.iobus_name = IOBUS_I2C_M1,
		.is_reset = 0,
	},
	[I2C_CH_MASTER2] = {
		.clk = 100,
		.IRQSTR = HwI2C_PORTCFG_BASE + 0x0C,
		.regs = (volatile struct tcc_i2c_regs *)HwI2C_MASTER2_BASE,
		.peri_name = PERI_I2C2,
		.iobus_name = IOBUS_I2C_M2,
		.is_reset = 0,
	},
	[I2C_CH_MASTER3] = {
		.clk = 100,
		.IRQSTR = HwI2C_PORTCFG_BASE + 0x0C,
		.regs = (volatile struct tcc_i2c_regs *)HwI2C_MASTER3_BASE,
		.peri_name = PERI_I2C3,
		.iobus_name = IOBUS_I2C_M3,
		.is_reset = 0,
	},
	/* SMU_I2C Controller */
	[I2C_CH_SMU] = {
		.clk = 100,									/* SMU_I2C prescale value */
		.IRQSTR = HwSMUI2C_BASE + 0x80,	/* SMU_I2C ICLK register address */
		.regs = (volatile struct tcc_i2c_regs *)HwSMUI2C_BASE,
	},
};

static int wait_intr(int i2c_name)
{
	volatile unsigned long cnt = 0;

	if (i2c_name < I2C_CH_SMU) {
#if 1	
		while (!(read_reg(i2c[i2c_name].IRQSTR) & (1<<i2c_name))) {
			cnt++;
			if (cnt > 100000) {
				printf("i2c-tcc: time out!\n");
				return -1;
			}
		}
#else
	/* check RxACK */
	while (1) {
		cnt++;
		if ((i2c[i2c_name].regs->SR & Hw0)) {
			if (!(i2c[i2c_name].regs->SR & Hw7)) {
				break;
			}
		}
		if (cnt > 100000) {
			printf("i2c-tcc: time out!\n");
			return -1;
		}
	}
#endif
	} else {
		/* SMU_I2C wait */
		while (1) {
			cnt++;
			if (!(i2c[i2c_name].regs->SR & Hw1)) break;
			if (cnt > 100000) {
				printf("smu-i2c-tcc: time out!\n");
				return -1;
			}
		}
		for (cnt = 0; cnt <15; cnt++);
	}

	return 0;
}

int i2c_xfer(unsigned char slave_addr, 
		unsigned char out_len, unsigned char* out_buf, 
		unsigned char in_len, unsigned char* in_buf, 
		int i2c_name)
{
	int ret;
	int i = 0;

	/* 
	 * WRITE 
	 */
	if (out_len > 0) {
		i2c[i2c_name].regs->TXR = slave_addr | I2C_WR;
		i2c[i2c_name].regs->CMD = Hw7 | Hw4;

		ret = wait_intr(i2c_name);
		if (ret != 0) return ret;
		BITSET(i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt

		for (i = 0; i < out_len; i++) {
			i2c[i2c_name].regs->TXR = out_buf[i];
			i2c[i2c_name].regs->CMD = Hw4;

			ret = wait_intr(i2c_name);
			if (ret != 0) return ret;
			BITSET(i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt
		}

		if(in_len <= 0)
		{
			i2c[i2c_name].regs->CMD = Hw6;

			ret = wait_intr(i2c_name);
			if (ret != 0) return ret;
		}

		BITSET(i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt
	}

	/* 
	 * READ
	 */
	if (in_len > 0) {
		i2c[i2c_name].regs->TXR = slave_addr | I2C_RD;
		i2c[i2c_name].regs->CMD = Hw7 | Hw4;

		ret = wait_intr(i2c_name);
		if (ret != 0) return ret;
		BITSET(i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt

		for (i = 0; i < in_len; i++) {
			//i2c[i2c_name].regs->CMD = Hw5 | Hw3;
			if (i == (in_len - 1)) 
    			i2c[i2c_name].regs->CMD = Hw5 | Hw3;
    		else
    			i2c[i2c_name].regs->CMD = Hw5;

			ret = wait_intr(i2c_name);
			if (ret != 0) return ret;

			BITSET( i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt

			in_buf[i] =(unsigned char)i2c[i2c_name].regs->RXR;
		}

		i2c[i2c_name].regs->CMD = Hw6;

		ret = wait_intr(i2c_name);
		if (ret != 0) return ret;
		BITSET( i2c[i2c_name].regs->CMD, Hw0); //Clear a pending interrupt
	}

	return 0;
}

static void i2c_reset(int i2c_name)
{
	if (!i2c[i2c_name].is_reset) {
		tcc_set_iobus_swreset(i2c[i2c_name].iobus_name, 1);
		tcc_set_iobus_swreset(i2c[i2c_name].iobus_name, 0);
		i2c[i2c_name].is_reset = 1;
	}
}

static void i2c_set_gpio(int i2c_name)
{
	switch (i2c_name)
	{
		case I2C_CH_MASTER0:
			#if defined(TARGET_BOARD_STB)
				#if defined(TARGET_TCC8935_STICK)
					//I2C[16] - GPIOE[14][15]
					//((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.bREG.MASTER0 = 16;
					BITCSET(((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.nREG, 0x000000FF, 16);
					gpio_config(TCC_GPE(14), GPIO_FN6|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPE(15), GPIO_FN6|GPIO_OUTPUT|GPIO_LOW);
				#else
					//I2C[29] - GPIOG[18][19]
					//((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.bREG.MASTER0 = 26;
					BITCSET(((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.nREG, 0x000000FF, 26);
					gpio_config(TCC_GPG(18), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPG(19), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				#endif
			#else
				if(HW_REV == 0x1000){
					//I2C[8] - GPIOB[9][10]
					BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 8);
					gpio_config(TCC_GPB(9), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPB(10), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
				}else if(HW_REV == 0x2000 || HW_REV == 0x3000){
					//I2C[18] - GPIOF[13][14]
					BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 18);
					gpio_config(TCC_GPF(13), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPF(14), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				}else if(HW_REV == 0x5000 || HW_REV == 0x5001 || HW_REV == 0x5002 || HW_REV == 0x5003){
					if (cpu_is_tcc8933s() || cpu_is_tcc8935s() || cpu_is_tcc8937s()) {
						BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 12);
						gpio_config(TCC_GPC(2), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
						gpio_config(TCC_GPC(3), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
					}
					else {
						BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 13);
						gpio_config(TCC_GPC(20), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
						gpio_config(TCC_GPC(21), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
					}
				}
			#endif
			break;
		case I2C_CH_MASTER1:
			#if defined(TARGET_BOARD_STB)
				#if defined(TARGET_TCC8935_STICK)
				#else
					//I2C[28] - GPIOADC[2][3]
					//((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.bREG.MASTER0 = 28;
					BITCSET(((PI2CPORTCFG)HwI2C_PORTCFG_BASE)->PCFG0.nREG, 0x0000FF00, 28<<8);
					gpio_config(TCC_GPADC(2), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPADC(3), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				#endif
			#else
				if(HW_REV == 0x1000){
					//I2C[21] - GPIOF[27][28]
					BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 21<<8);
					gpio_config(TCC_GPF(27), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPF(28), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				}else if(HW_REV == 0x2000 || HW_REV == 0x3000){
					//I2C[28] - GPIO_ADC[2][3]
					BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 28<<8);
					gpio_config(TCC_GPADC(2), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
					gpio_config(TCC_GPADC(3), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
				}else if(HW_REV == 0x5000 || HW_REV == 0x5001 || HW_REV == 0x5002 || HW_REV == 0x5003){
					if (cpu_is_tcc8933s() || cpu_is_tcc8935s() || cpu_is_tcc8937s()) {
						BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 25<<8);
						gpio_config(TCC_GPG(12), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
						gpio_config(TCC_GPG(13), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					}
					else {
						BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 22<<8);
						gpio_config(TCC_GPG(12), GPIO_FN14|GPIO_OUTPUT|GPIO_LOW);
						gpio_config(TCC_GPG(13), GPIO_FN14|GPIO_OUTPUT|GPIO_LOW);
					}
				}
			#endif
			break;
		case I2C_CH_MASTER2:
			if(HW_REV == 0x1000){
				//I2C[15] - GPIOC[30][31]
				BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 15<<16);
				gpio_config(TCC_GPC(30), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
				gpio_config(TCC_GPC(31), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
			}else if(HW_REV == 0x5000 || HW_REV == 0x5001 || HW_REV == 0x5002 || HW_REV == 0x5003){
				BITCSET(((PI2CPORTCFG)(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 18<<8);
				gpio_config(TCC_GPF(13), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				gpio_config(TCC_GPF(14), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
			}
			break;
		case I2C_CH_MASTER3:
			break;
		default:
			break;
	}
}

static void i2c_set_clock(int i2c_name, unsigned int i2c_clk_input_freq_khz)
{
	unsigned int prescale;

	prescale = i2c_clk_input_freq_khz / (i2c[i2c_name].clk * 5) - 1;
	i2c[i2c_name].regs->PRES = prescale;
	i2c[i2c_name].regs->CTRL = Hw7 | Hw6 | HwZERO;	/* start enable, stop enable, 8bit mode */
	BITSET(i2c[i2c_name].regs->CMD, Hw0);			/* clear pending interrupt */
#if 0
	printf("i2c%d SCK(%d) <-- input clk(%dKhz) prescale(%d)\n", 
			i2c_name, i2c[i2c_name].clk, i2c_clk_input_freq_khz, prescale)
#endif
}

static void i2c_enable(int i2c_name)
{
	int input_freq;

	tcc_set_peri(i2c[i2c_name].peri_name, ENABLE, 6000000);	/* I2C peri bus enable */
	tcc_set_iobus_pwdn(i2c[i2c_name].iobus_name, 0);				/* I2C io bus enable */
	input_freq = tcc_get_peri(i2c[i2c_name].peri_name);		/* get I2C bus clock */
	i2c_set_clock(i2c_name, (input_freq / 1000));
}

void i2c_init_early(void)
{
}

void i2c_init(void)
{
	int ch;

	/* I2C Controller */
	for (ch = I2C_CH_MASTER0; ch <= I2C_CH_MASTER3; ch++) {
		i2c_reset(ch);
		i2c_set_gpio(ch);
		i2c_enable(ch);
	}

	/* SMU_I2C */
	write_reg(0x80000000, i2c[I2C_CH_SMU].IRQSTR);
	i2c[I2C_CH_SMU].regs->CTRL = 0;
	i2c[I2C_CH_SMU].regs->PRES = i2c[I2C_CH_SMU].clk;
	BITSET(i2c[I2C_CH_SMU].regs->CTRL, Hw7|Hw6);
}
