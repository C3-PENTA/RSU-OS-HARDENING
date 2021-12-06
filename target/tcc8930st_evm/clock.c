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

#include <platform/reg_physical.h>
#include <platform/tcc_ckc.h>
#include <plat/cpu.h>
#include <clock.h>

/************************************************************
* Function    : clock_init()
* Description :
*    - increase fbus clock (1.2V or higher level)
************************************************************/
void clock_init(void)
{
	tcc_set_clkctrl( FBUS_CPU,    ENABLE, 850000000);	/*FBUS_CPU      850 MHz */	// 1.2V
#if defined(TARGET_TCC8935_STICK)
	#if defined(DRAM_DDR3)
	tcc_set_clkctrl( FBUS_MEM,    ENABLE, 512000000);	/*FBUS_MEM      514.5 MHz */
	#else
	tcc_set_clkctrl( FBUS_MEM,    ENABLE, 300000000);	/*FBUS_MEM      300 MHz */
	#endif
	tcc_set_clkctrl( FBUS_DDI,    ENABLE, 284000000);	/*FBUS_DDI      284.5 MHz */
	tcc_set_clkctrl( FBUS_GPU,   DISABLE, 274000000);	/*FBUS_GRP      328.1 MHz */
	tcc_set_clkctrl( FBUS_IO,     ENABLE, 153500000);	/*FBUS_IOB      153.8 MHz */
	tcc_set_clkctrl( FBUS_VBUS,  DISABLE, 256000000);	/*FBUS_VBUS     256.3 MHz */
	tcc_set_clkctrl( FBUS_CODA, DISABLE, 256000000);	/*FBUS_VCODEC   256.3 MHz */
	tcc_set_clkctrl( FBUS_HSIO,   ENABLE, 204700000);	/*FBUS_HSIO     213.6 MHz */
	tcc_set_clkctrl( FBUS_SMU,    ENABLE, 153500000);	/*FBUS_SMU      153.8 MHz */
	if (!cpu_is_tcc8933s() && !cpu_is_tcc8935s() && !cpu_is_tcc8937s()) {
		tcc_set_clkctrl( FBUS_G2D,   DISABLE, 284000000);	/*FBUS_G2D      284.5 MHz */
		tcc_set_clkctrl( FBUS_CMBUS, DISABLE, 204700000);	/*FBUS_CM3      213.6 MHz */
	}
#else  // TCC_CORE_1_1V_USE
	#if defined(DRAM_DDR3)
	tcc_set_clkctrl( FBUS_MEM,    ENABLE, 600000000);	/*FBUS_MEM      600 MHz */
	#else
	tcc_set_clkctrl( FBUS_MEM,    ENABLE, 300000000);	/*FBUS_MEM      300 MHz */
	#endif
	tcc_set_clkctrl( FBUS_DDI,    ENABLE, 333333334);	/*FBUS_DDI      333 MHz */
	tcc_set_clkctrl( FBUS_GPU,   DISABLE, 333333334);	/*FBUS_GRP      384 MHz */
	tcc_set_clkctrl( FBUS_IO,     ENABLE, 180000000);	/*FBUS_IOB      180 MHz */
	tcc_set_clkctrl( FBUS_VBUS,  DISABLE, 300000000);	/*FBUS_VBUS     300 MHz */
	tcc_set_clkctrl( FBUS_CODA, DISABLE, 300000000);	/*FBUS_VCODEC   300 MHz */
	tcc_set_clkctrl( FBUS_HSIO,   ENABLE, 250000000);	/*FBUS_HSIO     250 MHz */
	tcc_set_clkctrl( FBUS_SMU,    ENABLE, 180000000);	/*FBUS_SMU      180 MHz */
	if (!cpu_is_tcc8933s() && !cpu_is_tcc8935s() && !cpu_is_tcc8937s()) {
		tcc_set_clkctrl( FBUS_G2D,   DISABLE, 333333334);	/*FBUS_G2D      333 MHz */
		tcc_set_clkctrl( FBUS_CMBUS, DISABLE, 250000000);	/*FBUS_CM3      250 MHz */
	}
#endif  // TCC_CORE_1_1V_USE
}

/************************************************************
* Function    : clock_init_early()
* Description :
*    - set pll/peri-clock and
*    - set fbus clock to low (1.0V level)
************************************************************/
void clock_init_early(void)
{
	tcc_ckc_init();

	tcc_set_clkctrl( FBUS_IO,     ENABLE,   6000000);	/*FBUS_IO		6 MHz */
	tcc_set_clkctrl( FBUS_SMU,    ENABLE,   6000000);	/*FBUS_SMU		6 MHz */

	/* Set fixed pll values  pll0:cpu only, pll1:set by sdram */
	if (cpu_is_tcc8933s() || cpu_is_tcc8935s() || cpu_is_tcc8937s()) {
		tcc_set_pll(PLL_2,  ENABLE, 1000000000, PLLDIV_0);
		tcc_set_pll(PLL_3,  ENABLE,  720000000, PLLDIV_0);
		tcc_set_pll(PLL_4, DISABLE,  594000000, PLLDIV_0);	/* Not Used */
		tcc_set_pll(PLL_5, DISABLE,  720000000, PLLDIV_0);	/* Not Used */
	}
	else {
#if defined(TCC_CORE_1_1V_USE)
		tcc_set_pll(PLL_2,  ENABLE,  500000000, PLLDIV_0);
		tcc_set_pll(PLL_3,  ENABLE,  568000000, PLLDIV_0);
		tcc_set_pll(PLL_4,  ENABLE,  594000000, PLLDIV_0);
		tcc_set_pll(PLL_5,  ENABLE,  614000000, PLLDIV_0);
#else  // TCC_CORE_1_1V_USE
		tcc_set_pll(PLL_2,  ENABLE, 1000000000, PLLDIV_0);
		tcc_set_pll(PLL_3,  ENABLE,  768000000, PLLDIV_0);
		#if defined(TARGET_TCC8935_STICK)
		tcc_set_pll(PLL_4,  ENABLE,  548000000, PLLDIV_0);
		#else
		tcc_set_pll(PLL_4,  ENABLE,  594000000, PLLDIV_0);
		#endif
		tcc_set_pll(PLL_5,  ENABLE,  720000000, PLLDIV_0);
#endif  // TCC_CORE_1_1V_USE
	}
}
