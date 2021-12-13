/*
 * Copyright (c) 2013 Telechips, Inc.
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

#ifndef _DEV_PMIC_TCC270_H_
#define _DEV_PMIC_TCC270_H_

typedef enum {
	TCC270_ID_DCDC1 = 0,
	TCC270_ID_DCDC2,
	TCC270_ID_DCDC3,
	TCC270_ID_BOOST,
	TCC270_ID_LDO1,
	TCC270_ID_LDO2,
	TCC270_ID_LDO3,
	TCC270_ID_LDO4,
	TCC270_ID_LDO5,
	TCC270_ID_LDO6,
	TCC270_ID_LDO7,
	TCC270_ID_LDO8,
	TCC270_MAX
} tcc270_src_id;

extern void tcc270_init(void);
extern int tcc270_set_voltage(int type, unsigned int mV);
extern int tcc270_set_power(int type, int onoff);
extern int tcc270_output_control(tcc270_src_id id, int onoff);

#endif /* _DEV_PMIC_TCC270_H_ */

