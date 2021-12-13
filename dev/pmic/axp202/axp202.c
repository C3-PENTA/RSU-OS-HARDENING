/***************************************************************************************
*	FileName    : axp202.c
*	Description : Enhanced single Cell Li-Battery and Power System Management IC driver
****************************************************************************************
*
*	TCC Board Support Package
*	Copyright (c) Telechips, Inc.
*	ALL RIGHTS RESERVED
*
****************************************************************************************/

#include <platform/reg_physical.h>
#include <i2c.h>
#include <debug.h>

#if defined(AXP202_PMIC)
#include <dev/axp202.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct axp202_voltage_t {
	unsigned int uV;
	unsigned char val;
};

/* device address */
#define SLAVE_ADDR_AXP202	0x68


/********************************************************************
	I2C Command & Values
********************************************************************/
/* Power Control */
#define AXP202_POWER_STS_REG            0x00
#define AXP202_MODE_CHARGE_STS_REG      0x01
#define AXP202_OTG_VBUS_STS_REG         0x02
#define AXP202_POWER_OUT_CTRL_REG       0x12
#define AXP202_DCDC2_REG                0x23
#define AXP202_DCDC2_LDO3_DVS_REG       0x25
#define AXP202_DCDC3_REG                0x27
#define AXP202_LDO24_REG                0x28
#define AXP202_LDO3_REG                 0x29
#define AXP202_VBUS_IPSOUT_REG          0x30
#define AXP202_POWER_OFF_REG            0x31
#define AXP202_SHDN_BATT_CHGLED_REG     0x32
#define AXP202_CHARGING_CTRL1_REG       0x33
#define AXP202_CHARGING_CTRL2_REG       0x34
#define AXP202_BACKUP_BATT_CHG_REG      0x35
#define AXP202_PEK_REG                  0x36
#define AXP202_DCDC_WORK_FREQ_REG       0x37
#define AXP202_BATT_CHG_UNDER_TEMP_REG  0x38
#define AXP202_BATT_CHG_OVER_TEMP_REG   0x39
#define AXP202_IPSOUT_V_WARNING_1_REG   0x3A
#define AXP202_IPSOUT_V_WARNING_2_REG   0x3B
#define AXP202_BATT_DCHG_UNDER_TEMP_REG 0x3C
#define AXP202_BATT_DCHG_OVER_TEMP_REG  0x3D
#define AXP202_DCDC_WORK_MODE_REG       0x80
#define AXP202_ADC_ENABLE1_REG          0x82
#define AXP202_ADC_ENABLE2_REG          0x83
#define AXP202_ADC_TSPIN_REG            0x84
#define AXP202_ADC_INPUT_RANGE_REG      0x85
#define AXP202_TIMER_CTRL               0x8A
#define AXP202_VBUS_DET_SRP_FUNC_REG    0x8B
#define AXP202_OVER_TEMP_SHDN_REG       0x8F

/* GPIO control */
#define AXP202_LDO5_FUNC_SET_REG        0x90
#define AXP202_LDO5_REG                 0x91
#define AXP202_GPIO1_FUNC_REG           0x92
#define AXP202_GPIO2_FUNC_REG           0x93
#define AXP202_GPIO012_STS_REG          0x94
#define AXP202_GPIO3_FUNC_REG           0x95

/* Interrupt control */
#define AXP202_IRQ_EN1_REG              0x40
#define AXP202_IRQ_EN2_REG              0x41
#define AXP202_IRQ_EN3_REG              0x42
#define AXP202_IRQ_EN4_REG              0x43
#define AXP202_IRQ_EN5_REG              0x44
#define AXP202_IRQ_STS1_REG             0x48
#define AXP202_IRQ_STS2_REG             0x49
#define AXP202_IRQ_STS3_REG             0x4A
#define AXP202_IRQ_STS4_REG             0x4B
#define AXP202_IRQ_STS5_REG             0x4C

/* ADC data */
#define AXP202_ADC_ACIN_VOL_H_REG       0x56
#define AXP202_ADC_ACIN_VOL_L_REG       0x57
#define AXP202_ADC_ACIN_CUR_H_REG       0x58
#define AXP202_ADC_ACIN_CUR_L_REG       0x59
#define AXP202_ADC_VBUS_VOL_H_REG       0x5A
#define AXP202_ADC_VBUS_VOL_L_REG       0x5B
#define AXP202_ADC_VBUS_CUR_H_REG       0x5C
#define AXP202_ADC_VBUS_CUR_L_REG       0x5D
#define AXP202_ADC_BATT_VOL_H_REG       0x78
#define AXP202_ADC_BATT_VOL_L_REG       0x79

/* BAT Charge */
#define AXP202_BATT_CHARGE_CURRENT_H_REG 0x7A
#define AXP202_BATT_CHARGE_CURRENT_L_REG 0x7B
#define AXP202_BATT_DISCHARGE_CURRENT_H_REG 0x7C
#define AXP202_BATT_DISCHARGE_CURRENT_L_REG 0x7D

/* ADC enable values */
#define AXP202_ADC_1_BATT_VOL           0x80
#define AXP202_ADC_1_BATT_CUR           0x40
#define AXP202_ADC_1_ACIN_VOL           0x20
#define AXP202_ADC_1_ACIN_CUR           0x10
#define AXP202_ADC_1_VBUS_VOL           0x08
#define AXP202_ADC_1_VBUS_CUR           0x04
#define AXP202_ADC_1_APS_VOL            0x02
#define AXP202_ADC_1_TS_PIN             0x01

#define AXP202_ADC_2_TEMP_MON           0x80
#define AXP202_ADC_2_GPIO0              0x08
#define AXP202_ADC_2_GPIO1              0x04


/* Interrupt values */
#define AXP202_IRQ_1_ACIN_OVER          0x80
#define AXP202_IRQ_1_ACIN_INSERT        0x40
#define AXP202_IRQ_1_ACIN_REMOVE        0x20
#define AXP202_IRQ_1_VBUS_OVER          0x10
#define AXP202_IRQ_1_VBUS_INSERT        0x08
#define AXP202_IRQ_1_VBUS_REMOVE        0x04
#define AXP202_IRQ_1_VBUS_LOWER_VALID   0x02

#define AXP202_IRQ_2_BATT_INSERT        0x80
#define AXP202_IRQ_2_BATT_REMOVE        0x40
#define AXP202_IRQ_2_BATT_ACT           0x20
#define AXP202_IRQ_2_BATT_QUIT_ACT      0x10
#define AXP202_IRQ_2_CHARGING           0x08
#define AXP202_IRQ_2_CHARGE_FINISH      0x04
#define AXP202_IRQ_2_BATT_OVER          0x02
#define AXP202_IRQ_2_BATT_UNDER         0x01

#define AXP202_IRQ_3_INTERNAL_OVER      0x80
#define AXP202_IRQ_3_NOT_ENOUGH_CURR    0x40
#define AXP202_IRQ_3_DCDC2_UNDER        0x10
#define AXP202_IRQ_3_DCDC3_UNDER        0x08
#define AXP202_IRQ_3_LDO3_UNDER         0x04
#define AXP202_IRQ_3_SHORT_KEY          0x02
#define AXP202_IRQ_3_LONG_KEY           0x01

#define AXP202_IRQ_4_N_OE_ON            0x80
#define AXP202_IRQ_4_N_OE_OFF           0x40
#define AXP202_IRQ_4_VBUS_VALID         0x20
#define AXP202_IRQ_4_VBUS_INVLAID       0x10
#define AXP202_IRQ_4_VBUS_SESSION_AB    0x08
#define AXP202_IRQ_4_VBUS_SESSION_END   0x04
#define AXP202_IRQ_4_APS_UNDER_LEV1     0x02
#define AXP202_IRQ_4_APS_UNDER_LEV2     0x01

#define AXP202_IRQ_5_TIMEOUT            0x80
#define AXP202_IRQ_5_PEK_RISING         0x40
#define AXP202_IRQ_5_PEK_FALLING        0x20
#define AXP202_IRQ_5_GPIO3_INPUT        0x08
#define AXP202_IRQ_5_GPIO2_INPUT        0x04
#define AXP202_IRQ_5_GPIO1_INPUT        0x02
#define AXP202_IRQ_5_GPIO0_INPUT        0x01


/* initial setting values */
#define AXP202_ADC1 ( 0 \
		| AXP202_ADC_1_BATT_VOL \
		| AXP202_ADC_1_BATT_CUR \
		| AXP202_ADC_1_ACIN_VOL \
		| AXP202_ADC_1_ACIN_CUR \
		| AXP202_ADC_1_APS_VOL \
		| AXP202_ADC_1_TS_PIN \
		)
#define AXP202_ADC2 ( 0 \
		)

#define AXP202_IRQ1 ( 0 \
		| AXP202_IRQ_1_ACIN_OVER \
		| AXP202_IRQ_1_ACIN_INSERT \
		| AXP202_IRQ_1_ACIN_REMOVE \
		)
#define AXP202_IRQ2 ( 0 \
		| AXP202_IRQ_2_BATT_ACT \
		| AXP202_IRQ_2_BATT_QUIT_ACT \
		| AXP202_IRQ_2_CHARGING \
		| AXP202_IRQ_2_CHARGE_FINISH \
		| AXP202_IRQ_2_BATT_OVER \
		| AXP202_IRQ_2_BATT_UNDER \
		)
#define AXP202_IRQ3 ( 0 \
		| AXP202_IRQ_3_INTERNAL_OVER \
		| AXP202_IRQ_3_NOT_ENOUGH_CURR \
		| AXP202_IRQ_3_DCDC2_UNDER \
		| AXP202_IRQ_3_DCDC3_UNDER \
		| AXP202_IRQ_3_LDO3_UNDER \
		| AXP202_IRQ_3_SHORT_KEY \
		| AXP202_IRQ_3_LONG_KEY \
		)
#define AXP202_IRQ4 ( 0 \
		| AXP202_IRQ_4_APS_UNDER_LEV2 \
		)
#define AXP202_IRQ5 ( 0 \
		| AXP202_IRQ_5_TIMEOUT \
		)

/* Canging control 1 values */
#define AXP202_CHG_EN               0x80
#define AXP202_CHG_VOL_4_10V        0x00
#define AXP202_CHG_VOL_4_15V        0x20
#define AXP202_CHG_VOL_4_20V        0x40
#define AXP202_CHG_VOL_4_36V        0x60
#define AXP202_CHG_VOL_MASK         0x60
#define AXP202_CHG_OFF_CUR_10PER    0x00
#define AXP202_CHG_OFF_CUR_15PER    0x10
#define AXP202_CHG_OFF_CUR_MASK     0x10

/* DCDC voltage level */
static struct axp202_voltage_t dcdc_voltages[] = {
	{  700000, 0x00 }, {  725000, 0x01 }, {  750000, 0x02 }, {  775000, 0x03 },
	{  800000, 0x04 }, {  825000, 0x05 }, {  850000, 0x06 }, {  875000, 0x07 },
	{  900000, 0x08 }, {  925000, 0x09 }, {  950000, 0x0A }, {  975000, 0x0B },
	{ 1000000, 0x0C }, { 1025000, 0x0D }, { 1050000, 0x0E }, { 1075000, 0x0F }, // 16
	{ 1100000, 0x10 }, { 1125000, 0x11 }, { 1150000, 0x12 }, { 1175000, 0x13 },
	{ 1200000, 0x14 }, { 1225000, 0x15 }, { 1250000, 0x16 }, { 1275000, 0x17 },
	{ 1300000, 0x18 }, { 1325000, 0x19 }, { 1350000, 0x1A }, { 1375000, 0x1B },
	{ 1400000, 0x1C }, { 1425000, 0x1D }, { 1450000, 0x1E }, { 1475000, 0x1F }, // 32
	{ 1500000, 0x20 }, { 1525000, 0x21 }, { 1550000, 0x22 }, { 1575000, 0x23 },
	{ 1600000, 0x24 }, { 1625000, 0x25 }, { 1650000, 0x26 }, { 1675000, 0x27 },
	{ 1700000, 0x28 }, { 1725000, 0x29 }, { 1750000, 0x2A }, { 1775000, 0x2B },
	{ 1800000, 0x2C }, { 1825000, 0x2D }, { 1850000, 0x2E }, { 1875000, 0x2F }, // 48
	{ 1900000, 0x30 }, { 1925000, 0x31 }, { 1950000, 0x32 }, { 1975000, 0x33 },
	{ 2000000, 0x34 }, { 2025000, 0x35 }, { 2050000, 0x36 }, { 2075000, 0x37 },
	{ 2100000, 0x38 }, { 2125000, 0x39 }, { 2150000, 0x3A }, { 2175000, 0x3B },
	{ 2200000, 0x3C }, { 2225000, 0x3D }, { 2250000, 0x3E }, { 2275000, 0x3F }, // 64
	{ 2300000, 0x40 }, { 2325000, 0x41 }, { 2350000, 0x42 }, { 2375000, 0x43 },
	{ 2400000, 0x44 }, { 2425000, 0x45 }, { 2450000, 0x46 }, { 2475000, 0x47 },
	{ 2500000, 0x48 }, { 2525000, 0x49 }, { 2550000, 0x4A }, { 2575000, 0x4B },
	{ 2600000, 0x4C }, { 2625000, 0x4D }, { 2650000, 0x4E }, { 2675000, 0x4F }, // 80
	{ 2700000, 0x50 }, { 2725000, 0x51 }, { 2750000, 0x52 }, { 2775000, 0x53 },
	{ 2800000, 0x54 }, { 2825000, 0x55 }, { 2850000, 0x56 }, { 2875000, 0x57 },
	{ 2900000, 0x58 }, { 2925000, 0x59 }, { 2950000, 0x5A }, { 2975000, 0x5B },
	{ 3000000, 0x5C }, { 3025000, 0x5D }, { 3050000, 0x5E }, { 3075000, 0x5F }, // 96
	{ 3100000, 0x60 }, { 3125000, 0x61 }, { 3150000, 0x62 }, { 3175000, 0x63 },
	{ 3200000, 0x64 }, { 3225000, 0x65 }, { 3250000, 0x66 }, { 3275000, 0x67 },
	{ 3300000, 0x68 }, { 3325000, 0x69 }, { 3350000, 0x6A }, { 3375000, 0x6B },
	{ 3400000, 0x6C }, { 3425000, 0x6D }, { 3450000, 0x6E }, { 3475000, 0x6F }, // 112
	{ 3500000, 0x70 },
}; 
#define NUM_DCDC	ARRAY_SIZE(dcdc_voltages)
#define NUM_DCDC2	64	/* 700mA~2275mA */

static struct axp202_voltage_t ldo2_voltages[] = {
	{ 1800000, 0x00 }, { 1900000, 0x01 }, { 2000000, 0x02 }, { 2100000, 0x03 },
	{ 2200000, 0x04 }, { 2300000, 0x05 }, { 2400000, 0x06 }, { 2500000, 0x07 },
	{ 2600000, 0x08 }, { 2700000, 0x09 }, { 2800000, 0x0A }, { 2900000, 0x0B },
	{ 3000000, 0x0C }, { 3100000, 0x0D }, { 3200000, 0x0E }, { 3300000, 0x0F }, // 16
};
#define NUM_LDO2	ARRAY_SIZE(ldo2_voltages)

#define NUM_LDO3	64	/* 700mA~2275mA */

static struct axp202_voltage_t ldo4_voltages[] = {
	{ 1250000, 0x00 }, { 1300000, 0x01 }, { 1400000, 0x02 }, { 1500000, 0x03 },
	{ 1600000, 0x04 }, { 1700000, 0x05 }, { 1800000, 0x06 }, { 1900000, 0x07 },
	{ 2000000, 0x08 }, { 2500000, 0x09 }, { 2700000, 0x0A }, { 2800000, 0x0B },
	{ 3000000, 0x0C }, { 3100000, 0x0D }, { 3200000, 0x0E }, { 3300000, 0x0F }, // 16
};
#define NUM_LDO4	ARRAY_SIZE(ldo4_voltages)

#define NUM_LDO5	ARRAY_SIZE(ldo2_voltages)

static int axp202_initialized = 0;
static int axp202_i2c_ch = 0;
static int axp202_acin_det = 0;
static int axp202_charge_sts = 0;
static unsigned int axp202_acin_voltage_read_count = 20;


static unsigned char axp202_read(unsigned char cmd)
{
	unsigned char recv_data;
	i2c_xfer(SLAVE_ADDR_AXP202, 1, &cmd, 1, &recv_data, axp202_i2c_ch);
	return recv_data;
}

static void axp202_write(unsigned char cmd, unsigned char value)
{
	unsigned char send_data[2];
	send_data[0] = cmd;
	send_data[1] = value;
	i2c_xfer(SLAVE_ADDR_AXP202, 2, send_data, 0, 0, axp202_i2c_ch);
}

int axp202_battery_voltage(void)
{
	signed long data[2];
	int ret = 4200;

#if 0
	data[0] = axp202_read(AXP202_ADC_BATT_VOL_H_REG);
	data[1] = axp202_read(AXP202_ADC_BATT_VOL_L_REG);
	//dprintf(INFO,"%x %x \n", data[0], data[1]);
	ret = ((data[0]<<4)|(data[1]&0xF))&0xFFF;
#endif

//	dbg("%s: %dmV\n", __func__, ret);
	return ret;
}

int axp202_acin_detect(void)
{
	signed long data[2];

#if 0
	if (axp202_acin_voltage_read_count) {
		data[0] = axp202_read(AXP202_ADC_ACIN_VOL_H_REG);
		data[1] = axp202_read(AXP202_ADC_ACIN_VOL_L_REG);
		//dprintf(INFO,"%x %x \n", data[0], data[1]);
		if ((((data[0]<<4)|(data[1]&0xF))&0xFFF) > 1000)
			axp202_acin_det = 1;
		else
			axp202_acin_det = 0;
	}
	axp202_acin_voltage_read_count--;
#endif

//	dbg("%s: %d\n", __func__, axp202_acin_det);
	return axp202_acin_det;
}




void axp202_init(int i2c_ch)
{
	signed long data[2];
	axp202_i2c_ch = i2c_ch;
	axp202_initialized = 1;

	/* ADC enable 1 */
//	axp202_write(AXP202_ADC_EN1_REG, 0xab);

	data[0] = axp202_read(AXP202_ADC_BATT_VOL_H_REG);
	data[1] = axp202_read(AXP202_ADC_BATT_VOL_L_REG);
}

void axp202_power_off(void)
{
//	axp202_write(AXP202_POWER_OFF_REG, 0xC6);
}

static int axp202_output_control(axp202_src_id id, unsigned int onoff)
{
	unsigned char reg, old_value, value, bit;

	switch (id){
		case AXP202_ID_DCDC2:
			reg = AXP202_POWER_OUT_CTRL_REG;
			bit = 4;
			break;
		case AXP202_ID_DCDC3:
			reg = AXP202_POWER_OUT_CTRL_REG;
			bit = 1;
			break;
//		case AXP202_ID_LDO1:
//			break;;
		case AXP202_ID_LDO2:
			reg = AXP202_POWER_OUT_CTRL_REG;
			bit = 2;
			break;
		case AXP202_ID_LDO3:
			reg = AXP202_POWER_OUT_CTRL_REG;
			bit = 6;
			break;
		case AXP202_ID_LDO4:
			reg = AXP202_POWER_OUT_CTRL_REG;
			bit = 3;
			break;
		case AXP202_ID_LDO5:
			old_value = axp202_read(AXP202_LDO5_FUNC_SET_REG);
			if (onoff) {
				value = ((old_value & 0xF8) | 0x03);	// low noise LDO
			}
			else {
				value = ((old_value & 0xF8) | 0x00);	// Low output
				//value = ((old_value & 0xF8) | 0x06);	// Floating
			}
			dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x\n", __func__, id, onoff, reg, value);
			axp202_write(AXP202_LDO5_FUNC_SET_REG, value);
			return 0;
		default:
			return -1;
	}

	old_value = axp202_read(reg);
	if (onoff)
		value = (old_value | (1<<bit));
	else
		value = (old_value & ~(1<<bit));

	dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x\n", __func__, id, onoff, reg, value);
	axp202_write(reg, value);

	return 0;
}

static int axp202_dcdc_set_voltage(axp202_src_id id, unsigned int uV)
{
	unsigned char reg, value = 0;
	unsigned int i, max_num = 0;

	switch (id) {
		case AXP202_ID_DCDC2:
			reg = AXP202_DCDC2_REG;
			max_num = NUM_DCDC2;
			break;
		case AXP202_ID_DCDC3:
			reg = AXP202_DCDC3_REG;
			max_num = NUM_DCDC;
			break;
		default:
			return -1;
	}

	for (i = 0; i < max_num; i++) {
		if (dcdc_voltages[i].uV >= uV) {
			value = dcdc_voltages[i].val;
			break;
		}
	}

	if (i == max_num)
		return -1;

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	axp202_write(reg, value);
	axp202_output_control(id, 1);
	return 0;
}

static int axp202_ldo_set_voltage(axp202_src_id id, unsigned int uV)
{
	unsigned char reg, old_value, value;
	unsigned int i;

	switch (id) {
		case AXP202_ID_LDO2:
			reg = AXP202_LDO24_REG;
			old_value = axp202_read(reg);
			for (i = 0; i < NUM_LDO2; i++) {
				if (ldo2_voltages[i].uV >= uV) {
					value = ldo2_voltages[i].val;
					break;
				}
			}
			if (i == NUM_LDO2)
				return -1;
			value = (old_value&(0xFF&(~(0xF<<4)))) | (value<<4);
			break;
		case AXP202_ID_LDO3:
			reg = AXP202_LDO3_REG;
			for (i = 0; i < NUM_LDO3; i++) {
				if (dcdc_voltages[i].uV >= uV) {
					value = dcdc_voltages[i].val;
					break;
				}
			}
			if (i == NUM_LDO3)
				return -1;
			break;
		case AXP202_ID_LDO4:
			reg = AXP202_LDO24_REG;
			old_value = axp202_read(reg);
			for (i = 0; i < NUM_LDO4; i++) {
				if (ldo4_voltages[i].uV >= uV) {
					value = ldo4_voltages[i].val;
					break;
				}
			}
			if (i == NUM_LDO4)
				return -1;
			value = (old_value&(0xFF&(~(0xF)))) | value;
			break;
		case AXP202_ID_LDO5:
			reg = AXP202_LDO5_REG;
			old_value = axp202_read(reg);
			for (i = 0; i < NUM_LDO5; i++) {
				if (ldo2_voltages[i].uV >= uV) {
					value = ldo2_voltages[i].val;
					break;
				}
			}
			if (i == NUM_LDO5)
				return -1;
			value = (old_value&(0xFF&(~(0xF<<4)))) | (value<<4);
			break;
		default:
			return -1;
	}

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	axp202_write(reg, value);
	axp202_output_control(id, 1);

	return 0;
}

int axp202_set_voltage(axp202_src_id id, unsigned int mV)
{
	if (axp202_initialized == 0)
		return -1;

	/* power off */
	if (mV == 0)
		axp202_output_control(id, 0);
	else {
		switch (id){
			case AXP202_ID_DCDC2:
			case AXP202_ID_DCDC3:
				axp202_dcdc_set_voltage(id, mV*1000);
				break;
			case AXP202_ID_LDO1:
			case AXP202_ID_LDO2:
			case AXP202_ID_LDO3:
			case AXP202_ID_LDO4:
			case AXP202_ID_LDO5:
				axp202_ldo_set_voltage(id, mV*1000);
				break;
			default:
				return -1;
		}
	}

	return 0;
}

int axp202_set_power(axp202_src_id id, unsigned int onoff)
{
	if (axp202_initialized == 0)
		return -1;

	if (onoff)
		axp202_output_control(id, 1);
	else
		axp202_output_control(id, 0);

	return 0;
}
#endif

/************* end of file *************************************************************/
