/****************************************************************************
 *
 * Copyright (C) 2014 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions
 * andlimitations under the License.
 ****************************************************************************/

#include <dev/gpio.h>
#include <platform/gpio.h>
#include <plat/cpu.h>

/* Set all ports to output low except some ports in init_tlb[] */
#define GPIO_INIT_PORTCFG_USE

struct gpio_cfg {
	unsigned port;
	unsigned function;
	unsigned direction;
	unsigned pull;
};

/* Please keep the order: GPA, GPB, ,,, GPG, GPHDMI, GPSD3, GPSD0 */
struct gpio_cfg init_tbl[] = {
	{ TCC_GPB(26) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//FM1288 PWD
	{ TCC_GPC(8) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//UCS_UCS_PWR2_H
	{ TCC_GPC(10) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//TOUCH_PWR_ON
	//{ TCC_GPC(25) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//GPS Reset
	{ TCC_GPD(15) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },//MM_PCB_VER0
	{ TCC_GPD(16) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },//MM_PCB_VER1
	{ TCC_GPD(17) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },//MM_PCB_VER2
	{ TCC_GPD(18) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },//MM_PCB_VER3
	{ TCC_GPD(19) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },//MM_PCB_VER4
	{ TCC_GPD(20) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//BTWIFI_PWR_ON
	{ TCC_GPE(28) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//LCD RESET
	{ TCC_GPG(16) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//IPOD MODE0
	{ TCC_GPG(17) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH , GPIO_PULLDISABLE },//IPOD MODE1
};

#ifdef GPIO_INIT_PORTCFG_USE
static int tlb_idx = 0;
static void gpio_set_init_port(unsigned GPIO, int max_size)
{
	int i;
	for (i=0 ; i<max_size ; i++) {
		if (((GPIO+i) == init_tbl[tlb_idx].port) && (tlb_idx < (sizeof(init_tbl)/sizeof(init_tbl[0])))) {
			gpio_config(init_tbl[tlb_idx].port, 
				init_tbl[tlb_idx].function|init_tbl[tlb_idx].direction|init_tbl[tlb_idx].pull);
			tlb_idx++;
		}
		else
		{
			/* set default status */
			gpio_config(GPIO+i, GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW|GPIO_PULLDISABLE);
		}
	}
}
#endif

void gpio_init_early(void)
{
#ifdef GPIO_INIT_PORTCFG_USE
	gpio_set_init_port(GPIO_PORTA, 16);
	gpio_set_init_port(GPIO_PORTB, 30);
#ifdef USE_CM4_EARLY_CAM
	gpio_set_init_port(GPIO_PORTC, 29);
#else
        gpio_set_init_port(GPIO_PORTC, 30);
#endif
	gpio_set_init_port(GPIO_PORTD, 22);
	gpio_set_init_port(GPIO_PORTE, 32);
	gpio_set_init_port(GPIO_PORTF, 32);
	gpio_set_init_port(GPIO_PORTG, 20);

	gpio_set_init_port(GPIO_PORTSD0, 10);
#else
	int i;

	for (i=0 ; i<(sizeof(init_tbl)/sizeof(init_tbl[0])) ; i++)
		gpio_config(init_tbl[i].port, init_tbl[i].function|init_tbl[i].direction|init_tbl[i].pull);

#endif
}

#if defined(SECURITY_TPM)
void gpio_init_after(void)
{
    gpio_set_init_port(GPIO_PORTC, 22);
}
#endif
