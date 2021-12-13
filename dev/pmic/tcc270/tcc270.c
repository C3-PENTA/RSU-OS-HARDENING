/***************************************************************************************
*	FileName    : TCC270.c
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
#include <power.h>

#if defined(TCC270_PMIC)
#include <dev/tcc270.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct tcc270_voltage_t {
	unsigned int uV;
	unsigned char val;
};

/* device address */
#define SLAVE_ADDR_TCC270	0x68


/********************************************************************
	I2C Command & Values
********************************************************************/
//config & setting
#define TCC270_DEVICE_ID_REG 					0x00
#define TCC270_RESET_CONTROL_REG				0x15
#define TCC270_POK_TIME_SETTING_REG			0x19
#define TCC270_SHDN_CONTROL_REG				0x1A
#define TCC270_POWEROFF_CONTROL_REG			0x1B
#define TCC270_OFF_MODE_CONFIG_REG			0x5D

// DC/DC
#define TCC270_BUCK1_CONTROL_REG				0x08
#define TCC270_BUCK2_CONTROL_REG				0x09
#define TCC270_BUCK3_CONTROL_REG				0x0A
#define TCC270_BOOST_REG						0x0B
#define TCC270_BUCK_MODE_REG					0x0C
#define TCC270_VSYS_BUCK_ENABLE_REG			0x17	

// LDO
#define TCC270_LDO2_CONTROL_REG				0x0D
#define TCC270_LDO3_CONTROL_REG				0x0E
#define TCC270_LDO4_CONTROL_REG				0x0F
#define TCC270_LDO5_CONTROL_REG				0x10
#define TCC270_LDO6_CONTROL_REG				0x11
#define TCC270_LDO7_CONTROL_REG				0x12
#define TCC270_LDO1_8_CONTROL_REG				0x13
#define TCC270_LDO_MODE_REG					0x14
#define TCC270_LDO_ENABLE_REG					0x18

// GPIO
#define TCC270_GPIO0_FUNCTION_REG				0x1C
#define TCC270_GPIO1_FUNCTION_REG				0x1D
#define TCC270_GPIO2_FUNCTION_REG				0x1E	
#define TCC270_DCDC4_OCP_REG					0xA9

// EVENT & IRQ
#define TCC270_EVENT_STANBY_MODE_REG			0x16
#define TCC270_OFF_EVENT_STATUS_REG			0x20
#define TCC270_IRQ1_ENABLE_REG					0x30
#define TCC270_IRQ1_STATUS_REG					0x31
#define TCC270_IRQ2_ENABLE_REG					0x32
#define TCC270_IRQ2_STATUS_REG					0x33
#define TCC270_IRQ3_ENABLE_REG					0x34
#define TCC270_IRQ3_STATUS_REG					0x35
#define TCC270_IRQ4_ENABLE_REG					0x36
#define TCC270_IRQ4_STATUS_REG					0x37
#define TCC270_IRQ5_ENABLE_REG					0x38
#define TCC270_IRQ5_STATUS_REG					0x39
#define TCC270_IRQ_CONTROL_REG				0x50
#define TCC270_IRQ_FLG_REG						0x51

// CHARGE CONFIG
#define TCC270_CHGCONTROL1_REG				0x01
#define TCC270_CHGCONTROL2_REG				0x02
#define TCC270_CHGCONTROL3_REG				0x03
#define TCC270_CHGCONTROL4_REG				0x04
#define TCC270_CHGCONTROL5_REG				0x05
#define TCC270_CHGCONTROL6_REG				0x06
#define TCC270_CHGCONTROL7_REG				0x07

// VOLTAGE & CURRENT
#define TCC270_VBATS_HIGH_REG					0x58
#define TCC270_VBATS_LOW_REG					0x59
#define TCC270_INTEMP_HIGH_REG					0x5A
#define TCC270_INTEMP_LOW_REG					0x5B
#define TCC270_RSVD2_REG						0x5C
#define TCC270_AIN_VOLTAGE_HIGH_REG			0x5E
#define TCC270_AIN_VOLTAGE_LOW_REG			0x5F
#define TCC270_ACIN_VOLTAGE_HIGH_REG			0x64
#define TCC270_ACIN_VOLTAGE_LOW_REG			0x65
#define TCC270_VBUS_VOLTAGE_HIGH_REG			0x66
#define TCC270_VBUS_VOLTAGE_LOW_REG			0x67
#define TCC270_VSYS_VOLTAGE_HIGH_REG			0x68
#define TCC270_VSYS_VOLTAGE_LOW_REG			0x69
#define TCC270_GPIO0_VOLTAGE_HIGH_REG		0x6A
#define TCC270_GPIO0_VOLTAGE_LOW_REG			0x6B
#define TCC270_GPIO1_VOLTAGE_HIGH_REG		0x6C
#define TCC270_GPIO1_VOLTAGE_LOW_REG			0x6D
#define TCC270_GPIO2_VOLTAGE_HIGH_REG		0x6E
#define TCC270_GPIO2_VOLTAGE_LOW_REG			0x6F
#define TCC270_BUCK1_VOLTAGE_HIGH_REG		0x70
#define TCC270_BUCK1_VOLTAGE_LOW_REG			0x71
#define TCC270_BUCK2_VOLTAGE_HIGH_REG		0x72
#define TCC270_BUCK2_VOLTAGE_LOW_REG			0x73
#define TCC270_BUCK3_VOLTAGE_HIGH_REG		0x74
#define TCC270_BUCK3_VOLTAGE_LOW_REG			0x75
#define TCC270_VBAT_CURRENT_HIGH_REG			0x76
#define TCC270_VBAT_CURRENT_LOW_REG			0x77

// COULOMB COUNTER
#define TCC270_COULOMB_TIMER_HIGH_REG 		0x60
#define TCC270_COULOMB_TIMER_LOW_REG		0x61
#define TCC270_COULOMB_CHANNEL_HIGH_REG 	0x62
#define TCC270_COULOMB_CHANNEL_LOW_REG		0x63
#define TCC270_COULOMB_COUNTER_CHG_H_H		0x78
#define TCC270_COULOMB_COUNTER_CHG_H_L		0x79
#define TCC270_COULOMB_COUNTER_CHG_L_H		0x7A
#define TCC270_COULOMB_COUNTER_CHG_L_L		0x7B
#define TCC270_COULOMB_COUNTER_DISCHG_H_H		0x7C
#define TCC270_COULOMB_COUNTER_DISCHG_H_L		0x7D
#define TCC270_COULOMB_COUNTER_DISCHG_L_H		0x7E
#define TCC270_COULOMB_COUNTER_DISCHG_L_L		0x7F

// UNKNOWN
#define TCC270_RSVD1_REG						0x52
#define TCC270_VARLTMAX_REG					0x53
#define TCC270_VARLTMIN1_REG					0x54
#define TCC270_VARLTMIN2_REG					0x55
#define TCC270_TARLTMAX_REG					0x56
#define TCC270_TARLTMIN_REG					0x57



/* LDO1 voltage level */
static struct tcc270_voltage_t ldo1_voltages[] = {
	{ 1500, 0x00 }, { 1800, 0x01 }, { 2500, 0x02 }, { 3000, 0x03 },
	{ 3300, 0x04 }, { 3300, 0x05 }, { 3300, 0x06 }, { 3300, 0x07 },
};

#define NUM_LDO1	ARRAY_SIZE(ldo1_voltages)

static int tcc270_initialized = 0;
static int tcc270_i2c_ch = 0;
static int tcc270_acin_det = 0;
static unsigned int tcc270_acin_voltage_read_count = 20;


static unsigned char tcc270_read(unsigned char cmd)
{
	unsigned char recv_data;
	i2c_xfer(SLAVE_ADDR_TCC270, 1, &cmd, 1, &recv_data, tcc270_i2c_ch);
	return recv_data;
}

static void tcc270_write(unsigned char cmd, unsigned char value)
{
	unsigned char send_data[2];
	send_data[0] = cmd;
	send_data[1] = value;
	i2c_xfer(SLAVE_ADDR_TCC270, 2, send_data, 0, 0, tcc270_i2c_ch);
}

int tcc270_battery_voltage(void)
{
	signed long data[2];
	int ret = 4200;

	data[0] = tcc270_read(TCC270_VBATS_HIGH_REG);
	data[1] = tcc270_read(TCC270_VBATS_LOW_REG);
	ret = (data[0] << 8) | data[1];

//	dbg("%s: %dmV\n", __func__, ret);
	return ret;
}

int tcc270_acin_detect(void)
{
	unsigned char value;

	value = tcc270_read(TCC270_IRQ1_STATUS_REG);

	if(value & 0xBF)
		return 1;
	else
		return 0;

}

void tcc270_init(void)
{
	signed long data[2];
	unsigned char value;
	tcc270_i2c_ch = I2C_CH_MASTER0;
	tcc270_initialized = 1;

	value = tcc270_read(0x0);

	dprintf(INFO, "%s: id:0x%x\n", __func__, value);
}

void tcc270_power_off(void)
{
	int value;

	dprintf(INFO,"%s\n", __func__);	
	
	value = tcc270_read(TCC270_SHDN_CONTROL_REG);
	value = value | 0x80;
	tcc270_write(TCC270_SHDN_CONTROL_REG, value);
}

int tcc270_output_control(tcc270_src_id id, int onoff)
{
	unsigned char reg, old_value, value, bit, offset;

	switch (id){
		case TCC270_ID_DCDC1:
		case TCC270_ID_DCDC2:
		case TCC270_ID_DCDC3:
		case TCC270_ID_BOOST:
			reg = TCC270_VSYS_BUCK_ENABLE_REG;
			offset = 0;
			break;
//		case TCC270_ID_LDO1:
		case TCC270_ID_LDO2:
		case TCC270_ID_LDO3:
		case TCC270_ID_LDO4:
		case TCC270_ID_LDO5:
		case TCC270_ID_LDO6:
		case TCC270_ID_LDO7:
		case TCC270_ID_LDO8:
			reg = TCC270_LDO_ENABLE_REG;
			offset = 5;
			break;
		default:
			return -1;
	}

	old_value = tcc270_read(reg);
	if (onoff)
		value = (old_value | (1<<(id- offset)));
	else
		value = (old_value & ~(1<<(id-offset)));

	dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x\n", __func__, id, onoff, reg, value);
	tcc270_write(reg, value);

	return 0;
}

static int tcc270_dcdc_set_voltage(tcc270_src_id id, unsigned int uV)
{
	unsigned char reg, value = 0, old_value = 0;
	unsigned char step = 0, vrc = 0;
	unsigned int i, max = 0;

	switch (id) {
		case TCC270_ID_DCDC1:
			reg = TCC270_BUCK1_CONTROL_REG;
			max = 2275; step = 25; vrc = 2;
			break;
		case TCC270_ID_DCDC2:
			reg = TCC270_BUCK2_CONTROL_REG;
			max = 3500; step = 25; vrc = 1;
			break;
		case TCC270_ID_DCDC3:
			reg = TCC270_BUCK3_CONTROL_REG;
			max = 3500; step = 50; vrc = 2;
			break;
		default:
			return -1;
	}

	if (uV > max || uV < 700){
		dprintf(INFO,"Wrong BUCK%d value!\n", id);
		return -1;
	}

	old_value = tcc270_read(reg);
	value = ((uV - 700) / step) << vrc;
	value = value | (old_value & (vrc + vrc/2));

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x\n", __func__, id, uV, reg, value);
	tcc270_write(reg, value);
	tcc270_output_control(id, 1);
	return 0;
}

static int tcc270_boost_set_voltage(tcc270_src_id id, unsigned int uV)
{
	unsigned char value = 0, old_value = 0;
	const int min = 4500, max = 5500;

	if(uV < min || uV > max){
		dprintf(INFO,"Wrong boost value!\n");
		return -1;
	}

	old_value = tcc270_read(TCC270_BOOST_REG);
	value = (uV - min)/100 | (old_value & 0xF0);

	return 0;
}

static int tcc270_ldo_set_voltage(tcc270_src_id id, unsigned int uV)
{
	unsigned char reg, value = 0, old_value = 0;
	unsigned char step = 0, rsv = 0;
	unsigned int i, min, max, mask;

	switch (id) {
		case TCC270_ID_LDO1:
			reg = TCC270_LDO1_8_CONTROL_REG;	break;
		case TCC270_ID_LDO2:
			reg = TCC270_LDO2_CONTROL_REG;	break;
		case TCC270_ID_LDO3:
			reg = TCC270_LDO3_CONTROL_REG;	break;
		case TCC270_ID_LDO4:
			reg = TCC270_LDO4_CONTROL_REG;   break;			
		case TCC270_ID_LDO5:
			reg = TCC270_LDO5_CONTROL_REG;   break;						
		case TCC270_ID_LDO6:			
			reg = TCC270_LDO6_CONTROL_REG;	break;		
		case TCC270_ID_LDO7:			
			reg = TCC270_LDO7_CONTROL_REG;	break;	
		case TCC270_ID_LDO8:			
			reg = TCC270_LDO1_8_CONTROL_REG;	 		
			break;
		default:
			return -1;
	}

	switch(id){
		case TCC270_ID_LDO1:
			break;
		case TCC270_ID_LDO2:
		case TCC270_ID_LDO3:
			min = 700; max = 3500; step = 25; rsv = 0; mask = 0x7F;
			break;
		case TCC270_ID_LDO4:
		case TCC270_ID_LDO5:
		case TCC270_ID_LDO6:			
		case TCC270_ID_LDO7:			
		case TCC270_ID_LDO8:			
			min = 1000; max = 3300; step = 100; rsv = 3; mask = 0xF8;
			break;
		default:
			return -1;
	}

	old_value = tcc270_read(reg);

	if(id == TCC270_ID_LDO1){
		for (i = 0; i < NUM_LDO1; i++) {
			if (ldo1_voltages[i].uV >= uV) {
				value = ldo1_voltages[i].val;
				break;
			}
		}
		mask = 0x7;
		value = value | (old_value & ~mask);
	}
	else{
		if(uV > max || uV < min){
			dprintf(INFO,"Wrong ldo value!\n");
			return -1;
		}
		value = (((uV - min) / step) << rsv) | (old_value & ~mask);
	}

	dprintf(INFO, "%s: id:%d, uV:%d, reg:0x%x, value:0x%x, old_value:0x%x\n", __func__, id, uV, reg, value, old_value);
	tcc270_write(reg, value);
	tcc270_output_control(id, 1);

	return 0;
}

int tcc270_set_voltage(int type, unsigned int mV)
{
    tcc270_src_id id;

	if (tcc270_initialized == 0)
		return -1;

    switch(type)
    {
        case PWR_CPU:
        id = TCC270_ID_DCDC1;
        break;
        case PWR_CORE:
        id = TCC270_ID_DCDC2;
        break;
        case PWR_MEM:
        id = TCC270_ID_DCDC3;
        break;
        case PWR_IOD0:
        id = TCC270_ID_LDO1;
        break;
        case PWR_IOD1:
        id = TCC270_ID_LDO2;
        break;
        case PWR_IOD2:
        id = TCC270_ID_LDO3;
        break;
        case PWR_HDMI_12D:
        id = TCC270_ID_LDO7;
        break;
        case PWR_HDMI_33D:
        id = TCC270_ID_LDO8;
        break;
        case PWR_BOOST_5V:
        id = TCC270_ID_BOOST;
        break;
        default:
        break;
    }

	/* power off */
	if (mV == 0)
		tcc270_output_control(id, 0);
	else {
		switch (id){
			case TCC270_ID_DCDC1:
			case TCC270_ID_DCDC2:
			case TCC270_ID_DCDC3:
				tcc270_dcdc_set_voltage(id, mV);
				break;
			case TCC270_ID_BOOST:
				tcc270_boost_set_voltage(id, mV);
				break;
			case TCC270_ID_LDO1:
			case TCC270_ID_LDO2:
			case TCC270_ID_LDO3:
			case TCC270_ID_LDO4:
			case TCC270_ID_LDO5:
			case TCC270_ID_LDO6:
			case TCC270_ID_LDO7:
			case TCC270_ID_LDO8:				
				tcc270_ldo_set_voltage(id, mV);
				break;
			default:
				return -1;
		}
	}

	return 0;
}

int tcc270_set_power(int type, int onoff)
{
    tcc270_src_id id;

	if (tcc270_initialized == 0)
		return -1;

    switch(type)
    {
        case PWR_CPU:
        id = TCC270_ID_DCDC1;
        break;
        case PWR_CORE:
        id = TCC270_ID_DCDC2;
        break;
        case PWR_MEM:
        id = TCC270_ID_DCDC3;
        break;
        case PWR_IOD0:
        id = TCC270_ID_LDO1;
        break;
        case PWR_IOD1:
        id = TCC270_ID_LDO2;
        break;
        case PWR_IOD2:
        id = TCC270_ID_LDO3;
        break;
        case PWR_HDMI_12D:
        id = TCC270_ID_LDO7;
        break;
        case PWR_HDMI_33D:
        id = TCC270_ID_LDO8;
        break;
        case PWR_BOOST_5V:
        id = TCC270_ID_BOOST;
        break;
        default:
        break;
    }

	if (onoff)
		tcc270_output_control(id, 1);
	else
		tcc270_output_control(id, 0);

	return 0;
}
#endif


