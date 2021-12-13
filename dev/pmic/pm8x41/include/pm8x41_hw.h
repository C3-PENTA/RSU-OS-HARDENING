/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PM8x41_HW_H_
#define _PM8x41_HW_H_

/* SMBB Registers */
#define SMBB_MISC_BOOT_DONE                   0x1642

/* SMBB bit values */
#define BOOT_DONE_BIT                         7

#define REVID_REVISION4                       0x103

/* GPIO Registers */
#define GPIO_PERIPHERAL_BASE                  0xC000
/* Peripheral base address for GPIO_X */
#define GPIO_N_PERIPHERAL_BASE(x)            (GPIO_PERIPHERAL_BASE + ((x) - 1) * 0x100)

/* Register offsets within GPIO */
#define GPIO_STATUS                           0x08
#define GPIO_MODE_CTL                         0x40
#define GPIO_DIG_VIN_CTL                      0x41
#define GPIO_DIG_PULL_CTL                     0x42
#define GPIO_DIG_OUT_CTL                      0x45
#define GPIO_EN_CTL                           0x46

/* GPIO bit values */
#define PERPH_EN_BIT                          7
#define GPIO_STATUS_VAL_BIT                   0


/* PON Peripheral registers */
#define PON_PON_REASON1                       0x808
#define PON_INT_RT_STS                        0x810
#define PON_INT_SET_TYPE                      0x811
#define PON_INT_POLARITY_HIGH                 0x812
#define PON_INT_POLARITY_LOW                  0x813
#define PON_INT_LATCHED_CLR                   0x814
#define PON_INT_EN_SET                        0x815
#define PON_INT_LATCHED_STS                   0x818
#define PON_INT_PENDING_STS                   0x819
#define PON_RESIN_N_RESET_S1_TIMER            0x844  /* bits 0:3  : S1_TIMER */
#define PON_RESIN_N_RESET_S2_TIMER            0x845  /* bits 0:2  : S2_TIMER */
#define PON_RESIN_N_RESET_S2_CTL              0x846  /* bit 7: S2_RESET_EN, bit 0:3 : RESET_TYPE  */
#define PON_PS_HOLD_RESET_CTL                 0x85A  /* bit 7: S2_RESET_EN, bit 0:3 : RESET_TYPE  */
#define PON_PS_HOLD_RESET_CTL2                0x85B

/* PON Peripheral register bit values */
#define RESIN_ON_INT_BIT                      1
#define RESIN_BARK_INT_BIT                    4
#define S2_RESET_EN_BIT                       7

#define S2_RESET_TYPE_WARM                    0x1
#define PON_RESIN_N_RESET_S2_TIMER_MAX_VALUE  0x7

void pm8x41_reg_write(uint32_t addr, uint8_t val);
uint8_t pm8x41_reg_read(uint32_t addr);

/* SPMI Macros */
#define REG_READ(_a)        pm8x41_reg_read(_a)
#define REG_WRITE(_a, _v)   pm8x41_reg_write(_a, _v)

#define REG_OFFSET(_addr)   ((_addr) & 0xFF)
#define PERIPH_ID(_addr)    (((_addr) & 0xFF00) >> 8)
#define SLAVE_ID(_addr)     ((_addr) >> 16)

/* LDO voltage ranges */
#define NLDO_UV_MIN                           375000
#define NLDO_UV_MAX                           1537500
#define NLDO_UV_STEP                          12500
#define NLDO_UV_VMIN_LOW                      750000

#define PLDO_UV_VMIN_LOW                      750000
#define PLDO_UV_VMIN_MID                      1500000
#define PLDO_UV_VMIN_HIGH                     1750000

#define PLDO_UV_MIN                           1537500
#define PDLO_UV_MID                           3075000
#define PLDO_UV_MAX                           4900000
#define PLDO_UV_STEP_LOW                      12500
#define PLDO_UV_STEP_MID                      25000
#define PLDO_UV_STEP_HIGH                     50000

#define LDO_RANGE_SEL_BIT                     0
#define LDO_VSET_SEL_BIT                      0
#define LDO_VREG_ENABLE_BIT                   7
#define LDO_NORMAL_PWR_BIT                    7

#define LDO_RANGE_CTRL                        0x40
#define LDO_STEP_CTRL                         0x41
#define LDO_POWER_MODE                        0x45
#define LDO_EN_CTL_REG                        0x46

#define PLDO_TYPE                             0
#define NLDO_TYPE                             1

#define LDO(_name, _type, _base, _range, _step, _enable) \
{ \
	.name = _name, \
	.type = _type, \
	.base = _base, \
	.range_reg = _range, \
	.step_reg = _step, \
	.enable_reg = _enable, \
}

#endif
