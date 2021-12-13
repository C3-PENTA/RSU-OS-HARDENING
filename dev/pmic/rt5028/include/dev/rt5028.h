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

#ifndef _DEV_PMIC_RT5028_H_
#define _DEV_PMIC_RT5028_H_

typedef enum {
	RT5028_ID_BUCK1 = 0,	
	RT5028_ID_BUCK2,		
	RT5028_ID_BUCK3,		
	RT5028_ID_BUCK4,		
	RT5028_ID_LDO1,			// 4
	RT5028_ID_LDO2,
	RT5028_ID_LDO3,
	RT5028_ID_LDO4,
	RT5028_ID_LDO5,
	RT5028_ID_LDO6,
	RT5028_ID_LDO7,
	RT5028_ID_LDO8,
	RT5028_ID_MAX
} rt5028_src_id;

extern void rt5028_init(int i2c_ch);
extern int rt5028_set_voltage(rt5028_src_id id, unsigned int mV);
extern int rt5028_set_power(rt5028_src_id id, unsigned int onoff);

#endif /* _DEV_PMIC_AXP192_H_ */

