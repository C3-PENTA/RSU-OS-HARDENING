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
	tcc_set_clkctrl( FBUS_CPU0,    ENABLE, 1000000000);
	tcc_set_clkctrl( FBUS_CPU1,    ENABLE, 1000000000);
#if !defined(TSBM_ENABLE) && !defined(CONFIG_ARM_TRUSTZONE)
	tcc_set_clkctrl( FBUS_CMBUS,  DISABLE,  270000000);	/* 300MHz*0.9 */
#endif
//	tcc_set_clkctrl( FBUS_DDRPHY,  ENABLE,  933000000);	/* Do not change membus */
//	tcc_set_clkctrl( FBUS_MEM,     ENABLE,  416000000);	/* Do not change membus */
	tcc_set_clkctrl( FBUS_VBUS,   DISABLE,  360000000);	/* 400MHz*0.9 */
	tcc_set_clkctrl( FBUS_CODA,   DISABLE,  360000000);	/* 400MHz*0.9 */
#if !defined(TSBM_ENABLE)
	tcc_set_clkctrl( FBUS_HSIO,    ENABLE,  300000000);	/* 333MHz*0.9 */
#endif
	tcc_set_clkctrl( FBUS_SMU,     ENABLE,  180000000);	/* 200MHz*0.9 */
	tcc_set_clkctrl( FBUS_G2D,    DISABLE,  360000000);	/* 400MHz*0.9 */
	tcc_set_clkctrl( FBUS_DDI,     ENABLE,  360000000);	/* 400MHz*0.9 */
	tcc_set_clkctrl( FBUS_GPU,     ENABLE,  400000000);	/* 500MHz*0.8 */
	tcc_set_clkctrl( FBUS_IO,      ENABLE,  225000000);	/* 250MHz*0.9 */
	tcc_set_clkctrl( FBUS_HEVC_C, DISABLE,  360000000);	/* 400MHz*0.9 */
	tcc_set_clkctrl( FBUS_HEVC_V, DISABLE,  360000000);	/* 400MHz*0.9 */
	tcc_set_clkctrl( FBUS_HEVC_B, DISABLE,  360000000);	/* 400MHz*0.9 */
}

void clock_init_early(void)
{
	tcc_ckc_init();

	/* need to change cpu0/1 source.  pll6 -> pll0/pll1 */
	tcc_set_clkctrl( FBUS_CPU0,    ENABLE, 775200000);	/* 775.2MHz@0.9V */
	tcc_set_clkctrl( FBUS_CPU1,    ENABLE, 646000000);	/* 6460MHz@0.9V */

	/*
	 * pll0: cpu0(cortex-a15) only
	 * pll1: cpu1(cortex-a7) only
	 * pll2: memory only
	 * pll3: gpu only
	 */
//	tcc_set_pll(PLL_4,  ENABLE,  800000000, 3);	/* Do not set this pll. Memory bus is using this. */
	tcc_set_pll(PLL_5,  ENABLE, 1000000000, 4);
	tcc_set_pll(PLL_6,  ENABLE,  768000000, 2);
	tcc_set_pll(PLL_7,  ENABLE, 1080000000, 3);
}
