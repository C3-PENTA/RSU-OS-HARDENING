/*
 * Copyright (c) 2015 Telechips, Inc.
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

#ifndef _DEV_PMIC_DA9062_H_
#define _DEV_PMIC_DA9062_H_

typedef enum {
	DA9062_ID_BUCK1 = 0,
	DA9062_ID_BUCK2,
	DA9062_ID_BUCK3,
	DA9062_ID_BUCK4,
	DA9062_ID_LDO1,
	DA9062_ID_LDO2,
	DA9062_ID_LDO3,
	DA9062_ID_LDO4,
	DA9062_MAX
} da9062_src_id;

extern void da9062_init(void);
extern int da9062_set_voltage(int type, unsigned int mV);
extern unsigned int da9062_get_voltage(int type);
extern int da9062_set_power(int type, int onoff);
extern int da9062_output_control(da9062_src_id id, int onoff);

#endif /* _DEV_PMIC_DA9062_H_ */

