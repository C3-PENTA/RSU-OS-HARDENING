/***************************************************************************************
*	FileName    : axp192.c
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

//#define DUMP_RT5028 1
#if defined(RT5028_PMIC)
#include <dev/rt5028.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct rt5028_voltage_t {
	unsigned int uV;
	unsigned char val;
};

/* device address */
#define SLAVE_ADDR_RT5028	0x7E // SADDR connect to AGND

/* command registers */
#define RT5028_DEVICE_ID_REG				0x00
#define RT5028_BUCK1_CONTROL_REG			0x01
#define RT5028_BUCK2_CONTROL_REG			0x02
#define RT5028_BUCK3_CONTROL_REG			0x03
#define RT5028_BUCK4_CONTROL_REG			0x04
#define RT5028_VRC_SETTING_REG				0x05
#define RT5028_BUCK_MODE_REG				0x06
#define RT5028_LDO1_CONTROL_REG				0x07
#define RT5028_LDO2_CONTROL_REG				0x08
#define RT5028_LDO3_CONTROL_REG				0x09
#define RT5028_LDO4_CONTROL_REG				0x0A
#define RT5028_LDO5_CONTROL_REG				0x0B
#define RT5028_LDO6_CONTROL_REG				0x0C
#define RT5028_LDO7_CONTROL_REG				0x0D
#define RT5028_LDO8_CONTROL_REG				0x0E
#define RT5028_BUCK_ON_OFF_REG				0x12
#define RT5028_LDO_ON_OFF_REG				0x13

#define RT5028_SYN_CLOCK_SPREAD				0x34

/* BUCK voltage level */
static struct rt5028_voltage_t buck12_voltages[] = {
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
    { 1800000, 0x2C }, { 1800000, 0x2D }, { 1800000, 0x2E }, { 1800000, 0x2F }, // 48
    { 1800000, 0x30 }, { 1800000, 0x31 }, { 1800000, 0x32 }, { 1800000, 0x33 },
    { 1800000, 0x34 }, { 1800000, 0x35 }, { 1800000, 0x36 }, { 1800000, 0x37 },
    { 1800000, 0x38 }, { 1800000, 0x39 }, { 1800000, 0x3A }, { 1800000, 0x3B },
    { 1800000, 0x3C }, { 1800000, 0x3D }, { 1800000, 0x3E }, { 1800000, 0x3F }, // 64
}; 
#define NUM_BUCK12    ARRAY_SIZE(buck12_voltages) /* 700mV~1800mV */

static struct rt5028_voltage_t buck34_voltages[] = {
    {  700000, 0x00 }, {  750000, 0x01 }, {  800000, 0x02 }, {  850000, 0x03 },
    {  900000, 0x04 }, {  950000, 0x05 }, { 1000000, 0x06 }, { 1050000, 0x07 },
    { 1100000, 0x08 }, { 1150000, 0x09 }, { 1200000, 0x0A }, { 1250000, 0x0B },
    { 1300000, 0x0C }, { 1350000, 0x0D }, { 1400000, 0x0E }, { 1450000, 0x0F }, // 16
    { 1500000, 0x10 }, { 1550000, 0x11 }, { 1600000, 0x12 }, { 1650000, 0x13 },
    { 1700000, 0x14 }, { 1750000, 0x15 }, { 1800000, 0x16 }, { 1850000, 0x17 },
    { 1900000, 0x18 }, { 1950000, 0x19 }, { 2000000, 0x1A }, { 2050000, 0x1B },
    { 2100000, 0x1C }, { 2150000, 0x1D }, { 2200000, 0x1E }, { 2250000, 0x1F }, // 32
    { 2300000, 0x20 }, { 2350000, 0x21 }, { 2400000, 0x22 }, { 2450000, 0x23 },
    { 2500000, 0x24 }, { 2550000, 0x25 }, { 2600000, 0x26 }, { 2650000, 0x27 },
    { 2700000, 0x28 }, { 2750000, 0x29 }, { 2800000, 0x2A }, { 2850000, 0x2B },
    { 2900000, 0x2C }, { 2950000, 0x2D }, { 3000000, 0x2E }, { 3050000, 0x2F }, // 48
    { 3100000, 0x30 }, { 3150000, 0x31 }, { 3200000, 0x32 }, { 3250000, 0x33 },
    { 3300000, 0x34 }, { 3350000, 0x35 }, { 3400000, 0x36 }, { 3450000, 0x37 }, 
    { 3500000, 0x38 }, { 3550000, 0x39 }, { 3600000, 0x3A }, { 3600000, 0x3B }, 
    { 3600000, 0x3C }, { 3600000, 0x3D }, { 3600000, 0x3E }, { 3600000, 0x3F }, // 64
}; 
#define NUM_BUCK34    ARRAY_SIZE(buck34_voltages)	 /* 700mV~3600mV */


static struct rt5028_voltage_t ldo12378_voltages[] = {
    { 1600000, 0x00 }, { 1625000, 0x01 }, { 1650000, 0x02 }, { 1675000, 0x03 },
    { 1700000, 0x04 }, { 1725000, 0x05 }, { 1750000, 0x06 }, { 1775000, 0x07 },
    { 1800000, 0x08 }, { 1825000, 0x09 }, { 1850000, 0x0A }, { 1875000, 0x0B },
    { 1900000, 0x0C }, { 1925000, 0x0D }, { 1950000, 0x0E }, { 1975000, 0x0F }, // 16
    { 2000000, 0x10 }, { 2025000, 0x11 }, { 2050000, 0x12 }, { 2075000, 0x13 },
    { 2100000, 0x14 }, { 2125000, 0x15 }, { 2150000, 0x16 }, { 2175000, 0x17 },
    { 2200000, 0x18 }, { 2225000, 0x19 }, { 2250000, 0x1A }, { 2275000, 0x1B },
    { 2300000, 0x1C }, { 2325000, 0x1D }, { 2350000, 0x1E }, { 2375000, 0x1F }, // 32
    { 2400000, 0x20 }, { 2425000, 0x21 }, { 2450000, 0x22 }, { 2475000, 0x23 },
    { 2500000, 0x24 }, { 2525000, 0x25 }, { 2550000, 0x26 }, { 2575000, 0x27 },
    { 2600000, 0x28 }, { 2625000, 0x29 }, { 2650000, 0x2A }, { 2675000, 0x2B },
    { 2700000, 0x2C }, { 2725000, 0x2D }, { 2750000, 0x2E }, { 2775000, 0x2F }, // 48
    { 2800000, 0x30 }, { 2825000, 0x31 }, { 2850000, 0x32 }, { 2875000, 0x33 },
    { 2900000, 0x34 }, { 2925000, 0x35 }, { 2950000, 0x36 }, { 2975000, 0x37 },
    { 3000000, 0x38 }, { 3025000, 0x39 }, { 3050000, 0x3A }, { 3075000, 0x3B },
    { 3100000, 0x3C }, { 3125000, 0x3D }, { 3150000, 0x3E }, { 3175000, 0x3F }, // 64
    { 3200000, 0x40 }, { 3225000, 0x41 }, { 3250000, 0x42 }, { 3275000, 0x43 },
    { 3300000, 0x44 }, { 3325000, 0x45 }, { 3350000, 0x46 }, { 3375000, 0x47 },
    { 3400000, 0x48 }, { 3425000, 0x49 }, { 3450000, 0x4A }, { 3475000, 0x4B },
    { 3500000, 0x4C }, { 3525000, 0x4D }, { 3550000, 0x4E }, { 3575000, 0x4F }, // 80
    { 3600000, 0x50 }, { 3600000, 0x51 }, { 3600000, 0x52 }, { 3600000, 0x53 },
    { 3600000, 0x54 }, { 3600000, 0x55 }, { 3600000, 0x56 }, { 3600000, 0x57 },
    { 3600000, 0x58 }, { 3600000, 0x59 }, { 3600000, 0x5A }, { 3600000, 0x5B },
    { 3600000, 0x5C }, { 3600000, 0x5D }, { 3600000, 0x5E }, { 3600000, 0x5F }, // 96
    { 3600000, 0x60 }, { 3600000, 0x61 }, { 3600000, 0x62 }, { 3600000, 0x63 },
    { 3600000, 0x64 }, { 3600000, 0x65 }, { 3600000, 0x66 }, { 3600000, 0x67 },
    { 3600000, 0x68 }, { 3600000, 0x69 }, { 3600000, 0x6A }, { 3600000, 0x6B },
    { 3600000, 0x6C }, { 3600000, 0x6D }, { 3600000, 0x6E }, { 3600000, 0x6F }, // 112
    { 3600000, 0x70 }, { 3600000, 0x71 }, { 3600000, 0x72 }, { 3600000, 0x73 },
    { 3600000, 0x74 }, { 3600000, 0x75 }, { 3600000, 0x76 }, { 3600000, 0x77 },
    { 3600000, 0x78 }, { 3600000, 0x79 }, { 3600000, 0x7A }, { 3600000, 0x7B },
    { 3600000, 0x7C }, { 3600000, 0x7D }, { 3600000, 0x7E }, { 3600000, 0x7F },
};

#define NUM_LDO12378     ARRAY_SIZE(ldo12378_voltages)


static struct rt5028_voltage_t ldo456_voltages[] = {
    { 3000000, 0x00 }, { 3025000, 0x01 }, { 3050000, 0x02 }, { 3075000, 0x03 },
    { 3100000, 0x04 }, { 3125000, 0x05 }, { 3150000, 0x06 }, { 3175000, 0x07 },
    { 3200000, 0x08 }, { 3225000, 0x09 }, { 3250000, 0x0A }, { 3275000, 0x0B },
    { 3300000, 0x0C }, { 3325000, 0x0D }, { 3350000, 0x0E }, { 3375000, 0x0F }, // 16
    { 3400000, 0x10 }, { 3425000, 0x11 }, { 3450000, 0x12 }, { 3475000, 0x13 },
    { 3500000, 0x14 }, { 3525000, 0x15 }, { 3550000, 0x16 }, { 3575000, 0x17 },
    { 3600000, 0x18 }, { 3600000, 0x19 }, { 3600000, 0x1A }, { 3600000, 0x1B },
    { 3600000, 0x1C }, { 3600000, 0x1D }, { 3600000, 0x1E }, { 3600000, 0x1F }, // 32
    { 3600000, 0x20 }, { 3600000, 0x21 }, { 3600000, 0x22 }, { 3600000, 0x23 },
    { 3600000, 0x24 }, { 3600000, 0x25 }, { 3600000, 0x26 }, { 3600000, 0x27 },
    { 3600000, 0x28 }, { 3600000, 0x29 }, { 3600000, 0x2A }, { 3600000, 0x2B },
    { 3600000, 0x2C }, { 3600000, 0x2D }, { 3600000, 0x2E }, { 3600000, 0x2F }, // 48
    { 3600000, 0x30 }, { 3600000, 0x31 }, { 3600000, 0x32 }, { 3600000, 0x33 },
    { 3600000, 0x34 }, { 3600000, 0x35 }, { 3600000, 0x36 }, { 3600000, 0x37 },
    { 3600000, 0x38 }, { 3600000, 0x39 }, { 3600000, 0x3A }, { 3600000, 0x3B },
    { 3600000, 0x3C }, { 3600000, 0x3D }, { 3600000, 0x3E }, { 3600000, 0x3F }, // 64
    { 3600000, 0x40 }, { 3600000, 0x41 }, { 3600000, 0x42 }, { 3600000, 0x43 },
    { 3600000, 0x44 }, { 3600000, 0x45 }, { 3600000, 0x46 }, { 3600000, 0x47 },
    { 3600000, 0x48 }, { 3600000, 0x49 }, { 3600000, 0x4A }, { 3600000, 0x4B },
    { 3600000, 0x4C }, { 3600000, 0x4D }, { 3600000, 0x4E }, { 3600000, 0x4F }, // 80
    { 3600000, 0x50 }, { 3600000, 0x51 }, { 3600000, 0x52 }, { 3600000, 0x53 },
    { 3600000, 0x54 }, { 3600000, 0x55 }, { 3600000, 0x56 }, { 3600000, 0x57 },
    { 3600000, 0x58 }, { 3600000, 0x59 }, { 3600000, 0x5A }, { 3600000, 0x5B },
    { 3600000, 0x5C }, { 3600000, 0x5D }, { 3600000, 0x5E }, { 3600000, 0x5F }, // 96
    { 3600000, 0x60 }, { 3600000, 0x61 }, { 3600000, 0x62 }, { 3600000, 0x63 },
    { 3600000, 0x64 }, { 3600000, 0x65 }, { 3600000, 0x66 }, { 3600000, 0x67 },
    { 3600000, 0x68 }, { 3600000, 0x69 }, { 3600000, 0x6A }, { 3600000, 0x6B },
    { 3600000, 0x6C }, { 3600000, 0x6D }, { 3600000, 0x6E }, { 3600000, 0x6F }, // 112
    { 3600000, 0x70 }, { 3600000, 0x71 }, { 3600000, 0x72 }, { 3600000, 0x73 },
    { 3600000, 0x74 }, { 3600000, 0x75 }, { 3600000, 0x76 }, { 3600000, 0x77 },
    { 3600000, 0x78 }, { 3600000, 0x79 }, { 3600000, 0x7A }, { 3600000, 0x7B },
    { 3600000, 0x7C }, { 3600000, 0x7D }, { 3600000, 0x7E }, { 3600000, 0x7F },
};

#define NUM_LDO456     ARRAY_SIZE(ldo456_voltages)


static int rt5028_initialized = 0;
static int rt5028_i2c_ch = 0;

static unsigned char rt5028_read(unsigned char cmd)
{
	unsigned char recv_data;
	if(	i2c_xfer(SLAVE_ADDR_RT5028, 1, &cmd, 1, &recv_data, rt5028_i2c_ch) != 0) {
		printf("failed i2c_xfer\r\n");
	}
	return recv_data;
}

static unsigned char rt5028_read2(unsigned char id, unsigned char cmd)
{
	unsigned char recv_data;
	if(	i2c_xfer(id, 1, &cmd, 1, &recv_data, rt5028_i2c_ch) != 0) {
		printf("failed i2c_xfer\r\n");
	}
	return recv_data;
}


static void rt5028_write(unsigned char cmd, unsigned char value)
{
	unsigned char send_data[2];
	send_data[0] = cmd;
	send_data[1] = value;
	i2c_xfer(SLAVE_ADDR_RT5028, 2, send_data, 0, 0, rt5028_i2c_ch);
}

#if defined(DUMP_RT5028)
void rt5028_dump_buck(void)
{
	unsigned char reg, data;

	unsigned int i, uV=0, max_num = 0;

	static struct rt5028_voltage_t *buck_voltags;

	
	for(reg = RT5028_BUCK1_CONTROL_REG; reg <= RT5028_BUCK4_CONTROL_REG;reg++) {
		data = rt5028_read(reg);

		switch (reg) {
			case RT5028_BUCK1_CONTROL_REG:
				buck_voltags = buck12_voltages;
				max_num = NUM_BUCK12;
				break;
			case RT5028_BUCK2_CONTROL_REG:
				buck_voltags = buck12_voltages;
				max_num = NUM_BUCK12;
				break;
			case RT5028_BUCK3_CONTROL_REG:
				buck_voltags = buck34_voltages;
				max_num = NUM_BUCK34;
				break;
			case RT5028_BUCK4_CONTROL_REG:
				buck_voltags = buck34_voltages;
				max_num = NUM_BUCK34;
				break;
			default:
				return;
		}

		for (i = 0; i < max_num; i++) {
			if (buck_voltags[i].val == (data >> 2) ) {
				uV = buck_voltags[i].uV;
				break;
			}
		}
		printf("BUCK[%d] = 0x%04x (%d)uV\r\n", reg - RT5028_BUCK1_CONTROL_REG + 1,  data >> 2, uV);
	}
		
}

void rt5028_dump_ldo(void)
{
	unsigned char reg, data;

	unsigned int i, uV=0, max_num = 0;
	
	static struct rt5028_voltage_t *ldo_voltags;
	
	for(reg = RT5028_LDO2_CONTROL_REG; reg <= RT5028_LDO8_CONTROL_REG;reg++) {
		data = rt5028_read(reg);

		switch (reg) {
			case RT5028_LDO2_CONTROL_REG:
				ldo_voltags = ldo12378_voltages;
				max_num = NUM_LDO12378;
				break;
			case RT5028_LDO3_CONTROL_REG:
				ldo_voltags = ldo12378_voltages;
				max_num = NUM_LDO12378;
				break;
			case RT5028_LDO4_CONTROL_REG:
				ldo_voltags = ldo456_voltages;
				max_num = NUM_LDO456;
				break;
			case RT5028_LDO5_CONTROL_REG:
				ldo_voltags = ldo456_voltages;
				max_num = NUM_LDO456;
				break;
			case RT5028_LDO6_CONTROL_REG:
				ldo_voltags = ldo456_voltages;
				max_num = NUM_LDO456;
				break;
			case RT5028_LDO7_CONTROL_REG:
				ldo_voltags = ldo12378_voltages;
				max_num = NUM_LDO12378;
				break;
			case RT5028_LDO8_CONTROL_REG:
				ldo_voltags = ldo12378_voltages;
				max_num = NUM_LDO12378;
				break;
			default:
					return;
		}

		for (i = 0; i < max_num; i++) {
			if (ldo_voltags[i].val == data) {
				uV = ldo_voltags[i].uV;
				break;
			}
		}
		printf("LDO[%d] = 0x%04x (%d)uV\r\n", reg - RT5028_LDO1_CONTROL_REG + 1,  data, uV);
	}
		
}

#endif

void rt5028_init(int i2c_ch)
{
	char data, old_value, value;
	rt5028_i2c_ch = i2c_ch;
	rt5028_initialized = 1;

	printf("\r\n## RT5028 Early BUCK Mode Check ##\n");
	data = rt5028_read(RT5028_BUCK_MODE_REG);
	printf("RT5028 BUCK MODE=0x%x\r\n",data);
	data = rt5028_read(RT5028_BUCK1_CONTROL_REG);
	printf("RT5028 BUCK 1 val=0x%x\r\n",data);

	rt5028_write(RT5028_SYN_CLOCK_SPREAD,0);
//	data = rt5028_read(RT5028_SYN_CLOCK_SPREAD);
//	printf("RT5028 Syn-Clock Spread=0x%x\r\n", data);

	printf("######################################\r\n");
	#if defined(DUMP_RT5028)
	data = rt5028_read(RT5028_DEVICE_ID_REG);

	printf("\r\n");
	printf("rt5028_init RT5028 DEVID=0x%x\r\n", data);
	printf("\r\n");

	rt5028_dump_buck();
	rt5028_dump_ldo();
	#endif
#if 0	
	//set BUCK Mode
	old_value = rt5028_read(RT5028_BUCK_MODE_REG);
	value = old_value & 0x0F;
	printf("rt5028_init RT5028 BUCK mode(old=0x%x, new=0x%x)\r\n", old_value, value);
	rt5028_write(RT5028_BUCK_MODE_REG, value); //set Force PWM
	value = rt5028_read(RT5028_BUCK_MODE_REG);
	printf("RT5028 BUCK mode(0x%x)\r\n", value);
#endif
}


// ON OFF
static int rt5028_output_control(rt5028_src_id id, unsigned int onoff)
{
	unsigned char reg, old_value, value, bit;

	switch (id){	
		case RT5028_ID_BUCK1:
		case RT5028_ID_BUCK2:
		case RT5028_ID_BUCK3:
		case RT5028_ID_BUCK4:
			reg = RT5028_BUCK_ON_OFF_REG;
			bit = id-RT5028_ID_BUCK1;
			break;
		case RT5028_ID_LDO2:
		case RT5028_ID_LDO3:
		case RT5028_ID_LDO4:
		case RT5028_ID_LDO5:
		case RT5028_ID_LDO6:
		case RT5028_ID_LDO7:
		case RT5028_ID_LDO8:
			reg = RT5028_LDO_ON_OFF_REG;
			bit = id-RT5028_ID_LDO1;
			break;
		default:
			return -1;
	}

	old_value = rt5028_read(reg);
	if (onoff)
		value = (old_value | (1<<bit));
	else
		value = (old_value & ~(1<<bit));

	//dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x\n", __func__, id, onoff, reg, value);
	rt5028_write(reg, value);

	return 0;
}

static int rt5028_buck_set_voltage(rt5028_src_id id, unsigned int uV)
{
	unsigned char reg, old_value, value = 0;
	unsigned int i, max_num = 0;

	static struct rt5028_voltage_t *buck_voltags;
		
	switch (id) {
		case RT5028_ID_BUCK1:
			reg = RT5028_BUCK1_CONTROL_REG;
			buck_voltags = buck12_voltages;
			max_num = NUM_BUCK12;
			break;
		case RT5028_ID_BUCK2:
			reg = RT5028_BUCK2_CONTROL_REG;
			buck_voltags = buck12_voltages;
			max_num = NUM_BUCK12;
			break;
		case RT5028_ID_BUCK3:
			reg = RT5028_BUCK3_CONTROL_REG;
			buck_voltags = buck34_voltages;
			max_num = NUM_BUCK34;
			break;
		case RT5028_ID_BUCK4:
			reg = RT5028_BUCK4_CONTROL_REG;
			buck_voltags = buck34_voltages;
			max_num = NUM_BUCK34;
			break;
		default:
			return -1;
	}

	for (i = 0; i < max_num; i++) {
		if (buck_voltags[i].uV >= uV) {
			value = buck_voltags[i].val;
			break;
		}
	}

	if (i == max_num)
		return -1;
	
	old_value = rt5028_read(reg);
	value = (old_value & 0x3) | (value << 2);
	//dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	rt5028_write(reg, value);
	rt5028_output_control(id, 1);
	return 0;
}

static int rt5028_ldo_set_voltage(rt5028_src_id id, unsigned int uV)
{
	unsigned char reg, value;
	unsigned int i, max_num = 0;

	static struct rt5028_voltage_t *ldo_voltags;

	switch (id) {
		case RT5028_ID_LDO2:
			reg = RT5028_LDO2_CONTROL_REG;
			ldo_voltags = ldo12378_voltages;
			max_num = NUM_LDO12378;
			break;
		case RT5028_ID_LDO3:
			reg = RT5028_LDO3_CONTROL_REG;
			ldo_voltags = ldo12378_voltages;
			max_num = NUM_LDO12378;
			break;
		case RT5028_ID_LDO4:
			reg = RT5028_LDO4_CONTROL_REG;
			ldo_voltags = ldo456_voltages;
			max_num = NUM_LDO456;
			break;
		case RT5028_ID_LDO5:
			reg = RT5028_LDO5_CONTROL_REG;
			ldo_voltags = ldo456_voltages;
			max_num = NUM_LDO456;
			break;
		case RT5028_ID_LDO6:
			reg = RT5028_LDO6_CONTROL_REG;
			ldo_voltags = ldo456_voltages;
			max_num = NUM_LDO456;
			break;
		case RT5028_ID_LDO7:
			reg = RT5028_LDO7_CONTROL_REG;
			ldo_voltags = ldo12378_voltages;
			max_num = NUM_LDO12378;
			break;
		case RT5028_ID_LDO8:
			reg = RT5028_LDO8_CONTROL_REG;
			ldo_voltags = ldo12378_voltages;
			max_num = NUM_LDO12378;
			break;
		default:
			return -1;
	}

	for (i = 0; i < max_num; i++) {
		if (ldo_voltags[i].uV >= uV) {
			value = ldo_voltags[i].val;
			break;
		}
	}

	if (i == max_num)
		return -1;

	//dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	rt5028_write(reg, value);
	rt5028_output_control(id, 1);

	return 0;
}

int rt5028_set_voltage(rt5028_src_id id, unsigned int mV)
{
	//dprintf(INFO,"rt5028_set_voltage : %d\n", mV);
	if (rt5028_initialized == 0)
		return -1;

	/* power off */
	if (mV == 0)
		rt5028_output_control(id, 0);
	else {
		switch (id){
			case RT5028_ID_BUCK1:
			case RT5028_ID_BUCK2:
			case RT5028_ID_BUCK3:
			case RT5028_ID_BUCK4:
				rt5028_buck_set_voltage(id, mV*1000);
				break;
			case RT5028_ID_LDO2:
			case RT5028_ID_LDO3:
			case RT5028_ID_LDO4:
			case RT5028_ID_LDO5:
			case RT5028_ID_LDO6:
			case RT5028_ID_LDO7:
			case RT5028_ID_LDO8:
				rt5028_ldo_set_voltage(id, mV*1000);
				break;
			default:
				return -1;
		}
	}

	return 0;
}

int rt5028_set_power(rt5028_src_id id, unsigned int onoff)
{
	if (rt5028_initialized == 0)
		return -1;

	if (onoff)
		rt5028_output_control(id, 1);
	else
		rt5028_output_control(id, 0);

	return 0;
}
#endif

/************* end of file *************************************************************/
