/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
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

#ifndef _PM8X41_ADC_H_
#define _PM8X41_ADC_H_

#include <bits.h>
#include <stdint.h>

struct adc_conf {
	uint32_t base;
	uint8_t mode;
	uint16_t chan;
	uint8_t adc_param;
	uint8_t hw_set_time;
	uint8_t fast_avg;
	uint8_t calib_type;
};

#define CHAN_INIT(_base, _chan, _mode, _adc_param, _hw_set_time, _fast_avg, _calib_type) \
{\
	.base        = _base, \
	.chan        = _chan, \
	.mode        = _mode, \
	.adc_param   = _adc_param, \
	.hw_set_time = _hw_set_time, \
	.fast_avg    = _fast_avg, \
	.calib_type  = _calib_type, \
}

/* ADC1 Apps base */
#define VADC_USR1_BASE                   0x3100

/* ADC mode control */
#define VADC_MODE_CTRL                   0x40
#define VADC_MODE_NORMAL                 0x0
#define VADC_MODE_BIT_NORMAL             3

/* ADC channel control */
#define VADC_CHAN_SEL                    0x48

/* ADC dig param setup */
#define VADC_DIG_ADC_PARAM               0x50
#define VADC_DECIM_RATIO_VAL             0x1
#define VADC_DECIM_RATIO_SEL             2
#define VADC_CLK_SEL                     1

/* ADC HE settle time */
#define VADC_HW_SETTLE_TIME              0x51
#define VADC_HW_SET_TIME_SEL             7
#define HW_SET_DELAY_100US               0x1

/* Request VADC conversion */
#define VADC_CONV_REQ                    0x52
#define VADC_CON_REQ_BIT                 BIT(7)

/* ADC fast avg setup */
#define VADC_FAST_AVG                    0x5A
#define VADC_FAST_AVG_EN                 0x5B
#define FAST_AVG_SAMP_1                  0x0

/* VADC status */
#define VADC_STATUS                      0x08
#define VADC_STATUS_EOC                  0x1
#define VADC_STATUS_MASK                 0x3

/* VADC Enable */
#define VADC_EN_CTL                     0x46
#define VADC_CTL_EN_BIT                 BIT(7)

/* VADC result */
#define VADC_REG_DATA_MSB               0x61
#define VADC_REG_DATA_LSB               0x60

/* Channel IDs */
#define VADC_BAT_VOL_CHAN_ID            6
#define VREF_625_CHAN_ID                9
#define VREF_125_CHAN_ID                10
#define GND_REF_CHAN_ID                 14
#define VDD_VADC_CHAN_ID                15
#define VADC_BAT_CHAN_ID                49

/* Calibration type */
#define CALIB_ABS                       0
#define CALIB_RATIO                     1

/* VADC result range */
#define VADC_MIN_VAL                    0x6000
#define VADC_MAX_VAL                    0xA800

/* Data points needed for calibration */

/* Absolute calibration */
#define VREF_625_MV                     625000
#define OFFSET_GAIN_DNOM                3
#define OFFSET_GAIN_NUME                1

/* Ratiometric calibration */
#define VREF_18_V                       1800000

/* Current value for USB & BAT */
#define USB_CUR_100UA                   100000
#define USB_CUR_150UA                   150000
#define IUSB_MAX_REG                    0x1344

#define BAT_CUR_100UA                   100000
#define BAT_CUR_STEP                    50000
#define IBAT_MAX_REG                    0x1044

#define IUSB_MIN_UA                     100000
#define IBAT_MIN_UA                     100000
#define IBAT_MAX_UA                     3250000
#define IUSB_MAX_UA                     2500000

/* Function declations */
uint32_t pm8x41_adc_channel_read(uint16_t ch_num);
int pm8x41_iusb_max_config(uint32_t current);
int pm8x41_ibat_max_config(uint32_t current);

#endif /* _PM8X41_ADC_H_ */
