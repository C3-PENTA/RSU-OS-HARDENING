/***************************************************************************************
*	FileName    : rn5t614.c
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

#if defined(RN5T614_PMIC)
#include <dev/rn5t614.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct rn5t614_voltage_t {
	unsigned int uV;
	unsigned char val;
};

/* device address */
#define SLAVE_ADDR_RN5T614		0x64

/* registers */
#define RN5T614_PCCNT_REG		0x00
#define RN5T614_PCST_REG		0x01
#define RN5T614_VDCTRL_REG		0x02
#define RN5T614_LDOON_REG		0x03
#define RN5T614_LDO2DAC_REG		0x04
#define RN5T614_LDO3DAC_REG		0x05
#define RN5T614_LDO4DAC_REG		0x06
#define RN5T614_LDO5DAC_REG		0x07
#define RN5T614_LDO6DAC_REG		0x08
#define RN5T614_LDO7DAC_REG		0x09
#define RN5T614_LDO8DAC_REG		0x0A
#define RN5T614_DDCTL1_REG		0x10
#define RN5T614_DDCTL2_REG		0x11
#define RN5T614_RAMP1CTL_REG		0x12
#define RN5T614_RAMP2CTL_REG		0x13
#define RN5T614_DD1DAC_REG		0x14
#define RN5T614_DD2DAC_REG		0x15
#define RN5T614_CHGSTART_REG		0x20
#define RN5T614_FET1CNT_REG		0x21
#define RN5T614_FET2CNT_REG		0x22
#define RN5T614_TEST_REG		0x23
#define RN5T614_CMPSET_REG		0x24
#define RN5T614_SUSPEND_REG		0x25
#define RN5T614_CHGSTATE_REG		0x26
#define RN5T614_CHGEN1_REG		0x28
#define RN5T614_CHGIR1_REG		0x29
#define RN5T614_CHGMON1_REG		0x2A
#define RN5T614_CHGEN2_REG		0x2C
#define RN5T614_CHGIR2_REG		0x2D

/* DCDC voltage level */
static struct rn5t614_voltage_t dcdc_voltages[] = {
	{  900000, 0x00 }, {  912500, 0x01 }, {  925000, 0x02 }, {  937500, 0x03 },	// 4
	{  950000, 0x04 }, {  962500, 0x05 }, {  975000, 0x06 }, {  987500, 0x07 },	// 8
	{ 1000000, 0x08 }, { 1012500, 0x09 }, { 1025000, 0x0A }, { 1037500, 0x0B },	// 12
	{ 1050000, 0x0C }, { 1062500, 0x0D }, { 1075000, 0x0E }, { 1087500, 0x0F },	// 16
	{ 1100000, 0x10 }, { 1112500, 0x11 }, { 1125000, 0x12 }, { 1137500, 0x13 },	// 20
	{ 1150000, 0x14 }, { 1162500, 0x15 }, { 1175000, 0x16 }, { 1187500, 0x17 },	// 24
	{ 1200000, 0x18 }, { 1212500, 0x19 }, { 1225000, 0x1A }, { 1237500, 0x1B },	// 28
	{ 1250000, 0x1C }, { 1262500, 0x1D }, { 1275000, 0x1E }, { 1287500, 0x1F },	// 32
	{ 1300000, 0x20 }, { 1312500, 0x21 }, { 1325000, 0x22 }, { 1337500, 0x23 },	// 36
	{ 1350000, 0x24 }, { 1362500, 0x25 }, { 1375000, 0x26 }, { 1387500, 0x27 },	// 40
	{ 1400000, 0x28 }, { 1412500, 0x29 }, { 1425000, 0x2A }, { 1437500, 0x2B },	// 44
	{ 1450000, 0x2C }, { 1462500, 0x2D }, { 1475000, 0x2E }, { 1487500, 0x2F },	// 48
	{ 1500000, 0x30 }, { 1512500, 0x31 }, { 1525000, 0x32 }, { 1537500, 0x33 },	// 52
	{ 1550000, 0x34 }, { 1562500, 0x35 }, { 1575000, 0x36 }, { 1587500, 0x37 },	// 56
	{ 1600000, 0x38 }, { 1612500, 0x39 }, { 1625000, 0x3A }, { 1637500, 0x3B },	// 60
	{ 1650000, 0x3C }, { 1662500, 0x3D }, { 1675000, 0x3E }, { 1687500, 0x3F },	// 64
	{ 1700000, 0x40 }, { 1712500, 0x41 }, { 1725000, 0x42 }, { 1737500, 0x43 },	// 68
	{ 1750000, 0x44 }, { 1762500, 0x45 }, { 1775000, 0x46 }, { 1787500, 0x47 },	// 72
	{ 1800000, 0x48 },
}; 

#define NUM_DCDC    ARRAY_SIZE(dcdc_voltages)

static struct rn5t614_voltage_t ldo_voltages[8][8] = {
	{ {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 },
	  {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 } },	// LDO1 - Not Used
	{ {  900000, 0x00 }, { 1000000, 0x01 }, { 1100000, 0x02 }, { 1200000, 0x03 },
	  { 1300000, 0x04 }, {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 } },	// LDO2
	{ {  900000, 0x00 }, { 1000000, 0x01 }, { 1100000, 0x02 }, { 1200000, 0x03 },
	  { 1300000, 0x04 }, {       0, 0x00 }, {       0, 0x00 }, {       0, 0x00 } },	// LDO3
	{ { 1800000, 0x00 }, { 2500000, 0x01 }, { 2600000, 0x02 }, { 2800000, 0x03 },
	  { 2850000, 0x04 }, { 3000000, 0x05 }, { 3300000, 0x06 }, {       0, 0x00 } },	// LDO4
	{ { 1800000, 0x00 }, { 2500000, 0x01 }, { 2600000, 0x02 }, { 2800000, 0x03 },
	  { 2850000, 0x04 }, { 3000000, 0x05 }, { 3300000, 0x06 }, {       0, 0x00 } },	// LDO5
	{ { 1200000, 0x00 }, { 1800000, 0x01 }, { 2500000, 0x02 }, { 2600000, 0x03 },
	  { 2800000, 0x04 }, { 2850000, 0x05 }, { 3000000, 0x06 }, { 3300000, 0x07 } },	// LDO6
	{ { 1200000, 0x00 }, { 1800000, 0x01 }, { 2500000, 0x02 }, { 2600000, 0x03 },
	  { 2800000, 0x04 }, { 2850000, 0x05 }, { 3000000, 0x06 }, { 3300000, 0x07 } },	// LDO7
	{ {       0, 0x00 }, { 1800000, 0x01 }, { 2500000, 0x02 }, { 2600000, 0x03 },
	  { 2800000, 0x04 }, { 2850000, 0x05 }, { 3000000, 0x06 }, { 3300000, 0x07 } },	// LDO8
};
#define NUM_LDO     8

static int rn5t614_initialized = 0;
static int rn5t614_i2c_ch = 0;

static int rn5t614_read(unsigned char cmd, unsigned char *data)
{
	int ret;
	ret = i2c_xfer(SLAVE_ADDR_RN5T614, 1, &cmd, 1, data, rn5t614_i2c_ch);
	if (ret)  dprintf("%s: read failed.  cmd:0x%x, data:0x%x, result:0x%x\n", __func__, cmd, *data, ret);
	return ret;
}

static int rn5t614_write(unsigned char cmd, unsigned char value)
{
	unsigned char send_data[2];
	int ret;
	send_data[0] = cmd;
	send_data[1] = value;
	ret = i2c_xfer(SLAVE_ADDR_RN5T614, 2, send_data, 0, 0, rn5t614_i2c_ch);
	if (ret)  dprintf("%s: write failed.  cmd:0x%x, result:0x%x\n", __func__, cmd, ret);
	return ret;
}

void rn5t614_power_off(void)
{
    rn5t614_write(0x25, 0x00);
}

static int rn5t614_output_control(rn5t614_src_id id, unsigned int onoff)
{
	unsigned char reg, old_value, value, bit;

	if (id < RN5T614_ID_LDO1)
		return -1;
	else if (id == RN5T614_ID_LDO1)
		return -1;
	else if (id < RN5T614_ID_LDO8) {
		reg = RN5T614_LDOON_REG;
		bit = id - RN5T614_ID_LDO1;
	}
	else
		return -1;

	rn5t614_read(reg, &old_value);
	
	if (onoff)
		value = (old_value | (1<<bit));
	else
		value = (old_value & ~(1<<bit));

	dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x\n", __func__, id, onoff, reg, value);
	rn5t614_write(reg, value);

	return 0;
}

static int rn5t614_dcdc_set_voltage(rn5t614_src_id id, unsigned int uV)
{
	unsigned char reg, value;
	int i;

	switch (id) {
		case RN5T614_ID_DCDC1:
			reg = RN5T614_DD1DAC_REG;
			break;
		case RN5T614_ID_DCDC2:
			reg = RN5T614_DD2DAC_REG;
			break;
//		case RN5T614_ID_DCDC3:
//			break;
		default:
			return -1;
	}

	for (i = 0; i < NUM_DCDC; i++) {
		if (dcdc_voltages[i].uV >= uV) {
			value = dcdc_voltages[i].val;
			break;
		}
	}

	if (i == NUM_DCDC)
		return -1;

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	rn5t614_write(reg, value);

	return 0;
}

static int rn5t614_ldo_set_voltage(rn5t614_src_id id, unsigned int uV)
{
	unsigned char reg, value;
	int i;

	switch (id){
#if (0)
		case RN5T614_ID_DCDC1:
			break;
		case RN5T614_ID_DCDC2:
			break;
		case RN5T614_ID_DCDC3:
			break;
#endif
//		case RN5T614_ID_LDO1:
//			break;;
		case RN5T614_ID_LDO2:
			reg = RN5T614_LDO2DAC_REG;
			break;
		case RN5T614_ID_LDO3:
			reg = RN5T614_LDO3DAC_REG;
			break;
		case RN5T614_ID_LDO4:
			reg = RN5T614_LDO4DAC_REG;
			break;
		case RN5T614_ID_LDO5:
			reg = RN5T614_LDO5DAC_REG;
			break;
		case RN5T614_ID_LDO6:
			reg = RN5T614_LDO6DAC_REG;
			break;
		case RN5T614_ID_LDO7:
			reg = RN5T614_LDO7DAC_REG;
			break;
		case RN5T614_ID_LDO8:
			reg = RN5T614_LDO8DAC_REG;
			break;
		default:
			return -1;
	}

	for (i = 0; i < NUM_LDO; i++) {
		if (ldo_voltages[id-RN5T614_ID_LDO1][i].uV && (ldo_voltages[id-RN5T614_ID_LDO1][i].uV >= uV)) {
			value = ldo_voltages[id-RN5T614_ID_LDO1][i].val;
			break;
		}
	}

	if (i == NUM_LDO)
		return -1;

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	rn5t614_write(reg, value);

	return 0;
}

void rn5t614_init(int i2c_ch)
{
	unsigned char temp;
	rn5t614_i2c_ch = i2c_ch;
	rn5t614_initialized = 1;
}

int rn5t614_set_voltage(rn5t614_src_id id, unsigned int mV)
{
	if (rn5t614_initialized == 0)
		return -1;

	/* power off */
	if (mV == 0)
		rn5t614_output_control(id, 0);
	else {
		if (id < RN5T614_ID_LDO1)
			rn5t614_dcdc_set_voltage(id, mV*1000);
		else if (id < RN5T614_MAX)
			rn5t614_ldo_set_voltage(id, mV*1000);
		else
			return -1;
	}

	return 0;
}

int rn5t614_set_power(rn5t614_src_id id, unsigned int onoff)
{
	if (rn5t614_initialized == 0)
		return -1;

	if (onoff)
		rn5t614_output_control(id, 1);
	else
		rn5t614_output_control(id, 0);

	return 0;
}

#endif

/************* end of file *************************************************************/
