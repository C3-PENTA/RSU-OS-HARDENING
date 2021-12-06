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

#include <platform/reg_physical.h>
#include <platform/tcc_ckc.h>
#include <clock.h>
#include <power.h>
#include <i2c.h>

void clock_init(void)
{
	tcc_set_voltage( PWR_CPU0, ENABLE, 1100);  
	tcc_set_voltage( PWR_CORE, ENABLE, 1000+40);	/* 40mV: compensate for voltage drop of EVB */
	tcc_set_voltage(   PWR_GV, ENABLE, 1000+50);	/* 50mV: compensate for voltage drop of EVB */
	tcc_set_voltage(   PWR_VB, ENABLE, 1000+50);	/* 50mV: compensate for voltage drop of EVB */

	tcc_set_clkctrl( FBUS_CPU0,       ENABLE, 1000000000);
	tcc_set_clkctrl( FBUS_CPU1,      DISABLE,  800000000);	/* req:800MHz, real:768MHz = PLL1(768)/1 */
//	tcc_set_clkctrl( FBUS_MEM,        ENABLE,  600000000);	/* Do not change membus */
	tcc_set_clkctrl( FBUS_DDI,        ENABLE,  400000000);	/* req:400MHz, real:384MHz = PLL1(768)/2 */
	tcc_set_clkctrl( FBUS_GPU,        ENABLE,  525000000);	/* req:525MHz, real:500MHz = PLL0(1000)/2 */
	tcc_set_clkctrl( FBUS_IO,         ENABLE,  250000000);	/* req:250MHz, real:250MHz = PLL0(1000)/4 */
	tcc_set_clkctrl( FBUS_VBUS,      DISABLE,  360000000);	/* req:360MHz, real:360MHz = PLL2(1080)/3 */
	tcc_set_clkctrl( FBUS_CODA,      DISABLE,  360000000);	/* req:360MHz, real:360MHz = PLL2(1080)/3 */
//	tcc_set_clkctrl( FBUS_HSIO,       ENABLE,  333333334);  /* req:333MHz, real:333MHz = PLL0(1000)/3 */
	tcc_set_clkctrl( FBUS_HSIO,       ENABLE,  250000000);	/* req:250MHz, real:250MHz = PLL0(1000)/4 */ //for GMAC
	tcc_set_clkctrl( FBUS_SMU,        ENABLE,  200000000);  /* req:200MHz, real:200MHz = PLL0(1000)/5 */
	tcc_set_clkctrl( FBUS_G2D,       DISABLE,  800000000);  /* req:800MHz, real:540MHz = PLL2(1080)/2 */
#if defined(TSBM_ENABLE) || defined(CONFIG_ARM_TRUSTZONE) || defined(TZOW_INCLUDE) || defined(TCC_NSK_ENABLE)    
	tcc_set_clkctrl( FBUS_CMBUS,      ENABLE,  300000000);  /* req:300MHz, real:270MHz = PLL2(1080)/4 */
#else
	tcc_set_clkctrl( FBUS_CMBUS,     DISABLE,  300000000);  /* req:300MHz, real:270MHz = PLL2(1080)/4 */
#endif
	tcc_set_clkctrl( FBUS_HEVC_VCE,  DISABLE,  360000000);	/* req:360MHz, real:360MHz = PLL2(1080)/3 */
	tcc_set_clkctrl( FBUS_HEVC_VCPU, DISABLE,  360000000);	/* req:360MHz, real:360MHz = PLL2(1080)/3 */
	tcc_set_clkctrl( FBUS_HEVC_BPU,  DISABLE,  360000000);	/* req:360MHz, real:360MHz = PLL2(1080)/3 */
}

void clock_init_early(void)
{
	tcc_ckc_init();

	/* change clock sourt to XIN before change PLL values. */
	tcc_set_clkctrl(FBUS_IO,   ENABLE, XIN_CLK_RATE);
	tcc_set_clkctrl(FBUS_SMU,  ENABLE, XIN_CLK_RATE);
	tcc_set_clkctrl(FBUS_HSIO, ENABLE, XIN_CLK_RATE);

	/* need to change cpu0/1 source.  pll6 -> pll0/pll1 */
	tcc_set_clkctrl( FBUS_CPU0,    ENABLE, 540000000);	/* 600MHz@0.9V */

	/*
	 * pll3: cpu(cortex-a7) only
	 * pll4: memory only
	 */
	tcc_set_pll(PLL_0,  ENABLE, 1000000000, 25);
	tcc_set_pll(PLL_1,  ENABLE,  768000000, 3);
	tcc_set_pll(PLL_2,  ENABLE, 1080000000, 3);	/* for setting 360MHz at VBUS */
//	tcc_set_pll(PLL_2,  ENABLE, 1188000000, 3);	/* for supporting component/composite */
}
