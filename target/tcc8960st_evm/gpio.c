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

/* Set all ports to output low except some ports in init_tlb[] */
//#define GPIO_INIT_PORTCFG_USE

struct gpio_cfg {
	unsigned port;
	unsigned function;
	unsigned direction;
	unsigned pull;
};

/* Please keep the order: GPA, GPB, ,,, GPG, GPHDMI, GPSD3, GPSD0 */
struct gpio_cfg init_tbl[] = {
#if defined(TARGET_TCC8960_STB)
	{ TCC_GPB(31), GPIO_FN0, GPIO_OUTPUT|GPIO_LOW,  GPIO_PULLDISABLE },		// VE_RESET 

	{ TCC_GPC(20)  ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },	// USB_H_20_V

        { TCC_GPE(0) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // CPU0 Power 1.10V
        { TCC_GPE(1) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // CPU0 Power 1.10V

        { TCC_GPE(2) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // CPU1 Power 1.10V
        { TCC_GPE(3) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // CPU1 Power 1.10V

        //{ TCC_GPE(4) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW,  GPIO_PULLDISABLE },     // Core 1.10V
        //{ TCC_GPE(5) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // Core 1.10V

        { TCC_GPE(6) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },      // VB/GB : 1.10V
        { TCC_GPE(7) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // VB/GB : 1.10V

	{ TCC_GPE(8)  ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },	// SLP_PWRCTL 
	{ TCC_GPE(10) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },	// DRAM1 Power 

	{ TCC_GPE(26)  ,GPIO_FN0 , GPIO_INPUT, GPIO_PULLDISABLE },		// IR_WAKE for IR_Sensor Input.

	{ TCC_GPHDMI(0) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //CEC
	{ TCC_GPHDMI(1) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //HPD
	{ TCC_GPHDMI(2) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //I2C_SDA
	{ TCC_GPHDMI(3) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //I2C_SCL

#endif

#if defined(TARGET_TCC8963_STB)
	{ TCC_GPB(31), GPIO_FN0, GPIO_OUTPUT|GPIO_LOW,  GPIO_PULLDISABLE },		// VE_RESET 

	{ TCC_GPC(20)  ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },	// USB_H_20_V

        { TCC_GPD(21) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // CPU0 Power 1.10V
        { TCC_GPD(22) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // CPU0 Power 1.10V

        { TCC_GPD(23) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // CPU1 Power 1.10V
        { TCC_GPD(24) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // CPU1 Power 1.10V

        //{ TCC_GPD(25) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW,  GPIO_PULLDISABLE },     // Core 1.10V
        //{ TCC_GPD(26) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },     // Core 1.10V

        { TCC_GPD(27) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },      // VB/GB : 1.10V
        { TCC_GPD(28) ,GPIO_FN0 , GPIO_OUTPUT|GPIO_LOW, GPIO_PULLDISABLE },     // VB/GB : 1.10V

	{ TCC_GPD(31)  ,GPIO_FN0 , GPIO_OUTPUT|GPIO_HIGH, GPIO_PULLDISABLE },	// SLP_PWRCTL 

	{ TCC_GPE(26)  ,GPIO_FN0 , GPIO_INPUT, GPIO_PULLDISABLE },		// IR_WAKE for IR_Sensor Input.

	{ TCC_GPHDMI(0) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //CEC
	{ TCC_GPHDMI(1) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //HPD
	{ TCC_GPHDMI(2) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //I2C_SDA
	{ TCC_GPHDMI(3) ,GPIO_FN0 , GPIO_INPUT , GPIO_PULLDISABLE },  //I2C_SCL

#endif
};

static int tlb_idx = 0;

#ifdef GPIO_INIT_PORTCFG_USE
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
	gpio_set_init_port(GPIO_PORTA, 32);
	gpio_set_init_port(GPIO_PORTB, 32);
	gpio_set_init_port(GPIO_PORTC, 32);
	gpio_set_init_port(GPIO_PORTD, 32);
	gpio_set_init_port(GPIO_PORTE, 32);
	gpio_set_init_port(GPIO_PORTF, 32);
	gpio_set_init_port(GPIO_PORTG, 20);
	gpio_set_init_port(GPIO_PORTHDMI, 4);
	gpio_set_init_port(GPIO_PORTSD3, 10);
	gpio_set_init_port(GPIO_PORTSD0, 6);
#else
	int i;
	for (i=0 ; i<(sizeof(init_tbl)/sizeof(init_tbl[0])) ; i++)
		gpio_config(init_tbl[i].port, init_tbl[i].function|init_tbl[i].direction|init_tbl[i].pull);
#endif
}
