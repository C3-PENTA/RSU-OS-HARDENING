/***************************************************************************************
*	FileName    : da9062.c
*	Description : Power System Management IC driver
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

#if defined(DA9062_PMIC)
#include <dev/da9062.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct tcc270_voltage_t {
	unsigned int uV;
	unsigned char val;
};

/********************************************************************
	I2C Command & Values
********************************************************************/
/* device address */
#define SLAVE_ADDR_DA9062   0xb0

#define DA9062AA_VBUCK2_A       0x0A3
#define DA9062AA_VBUCK1_A       0x0A4
#define DA9062AA_VBUCK4_A       0x0A5
#define DA9062AA_VBUCK3_A       0x0A7

#define DA9062AA_BUCK2_CFG      0x09D
#define DA9062AA_BUCK1_CFG      0x09E
#define DA9062AA_BUCK4_CFG      0x09F
#define DA9062AA_BUCK3_CFG      0x0A0

/* DA9062AA_VBUCK2_A = 0x0A3 */
#define DA9062AA_VBUCK2_A_SHIFT     0
#define DA9062AA_VBUCK2_A_MASK      (0x7f << 0)
#define DA9062AA_BUCK2_SL_A_SHIFT   7
#define DA9062AA_BUCK2_SL_A_MASK    (0x01 << 7)

/* DA9062AA_VBUCK1_A = 0x0A4 */
#define DA9062AA_VBUCK1_A_SHIFT     0
#define DA9062AA_VBUCK1_A_MASK      (0x7f << 0)
#define DA9062AA_BUCK1_SL_A_SHIFT   7
#define DA9062AA_BUCK1_SL_A_MASK    (0x01 << 7)

/* DA9062AA_VBUCK4_A = 0x0A5 */
#define DA9062AA_VBUCK4_A_SHIFT     0
#define DA9062AA_VBUCK4_A_MASK      (0x7f << 0)
#define DA9062AA_BUCK4_SL_A_SHIFT   7
#define DA9062AA_BUCK4_SL_A_MASK    (0x01 << 7)

/* DA9062AA_VBUCK3_A = 0x0A7 */
#define DA9062AA_VBUCK3_A_SHIFT     0
#define DA9062AA_VBUCK3_A_MASK      (0x7f << 0)
#define DA9062AA_BUCK3_SL_A_SHIFT   7
#define DA9062AA_BUCK3_SL_A_MASK    (0x01 << 7)

/* DA9062AA_BUCK2_CFG = 0x09D */
#define DA9062AA_BUCK2_PD_DIS_SHIFT 5
#define DA9062AA_BUCK2_PD_DIS_MASK  (0x01 << 5)
#define DA9062AA_BUCK2_MODE_SHIFT   6
#define DA9062AA_BUCK2_MODE_MASK    (0x03 << 6)

/* DA9062AA_BUCK1_CFG = 0x09E */
#define DA9062AA_BUCK1_PD_DIS_SHIFT 5
#define DA9062AA_BUCK1_PD_DIS_MASK  (0x01 << 5)
#define DA9062AA_BUCK1_MODE_SHIFT   6
#define DA9062AA_BUCK1_MODE_MASK    (0x03 << 6)

/* DA9062AA_BUCK4_CFG = 0x09F */
#define DA9062AA_BUCK4_VTTR_EN_SHIFT    3
#define DA9062AA_BUCK4_VTTR_EN_MASK (0x01 << 3)
#define DA9062AA_BUCK4_VTT_EN_SHIFT 4
#define DA9062AA_BUCK4_VTT_EN_MASK  (0x01 << 4)
#define DA9062AA_BUCK4_PD_DIS_SHIFT 5
#define DA9062AA_BUCK4_PD_DIS_MASK  (0x01 << 5)
#define DA9062AA_BUCK4_MODE_SHIFT   6
#define DA9062AA_BUCK4_MODE_MASK    (0x03 << 6)

/* DA9062AA_BUCK3_CFG = 0x0A0 */
#define DA9062AA_BUCK3_PD_DIS_SHIFT 5
#define DA9062AA_BUCK3_PD_DIS_MASK  (0x01 << 5)
#define DA9062AA_BUCK3_MODE_SHIFT   6
#define DA9062AA_BUCK3_MODE_MASK    (0x03 << 6)

static int da9062_initialized = 0;
static int da9062_i2c_ch = 0;

static unsigned char da9062_read(unsigned char cmd)
{
	unsigned char recv_data;
	i2c_xfer(SLAVE_ADDR_DA9062, 1, &cmd, 1, &recv_data, da9062_i2c_ch);
	return recv_data;
}

static void da9062_write(unsigned char cmd, unsigned char value)
{
	unsigned char send_data[2];
	send_data[0] = cmd;
	send_data[1] = value;
	i2c_xfer(SLAVE_ADDR_DA9062, 2, send_data, 0, 0, da9062_i2c_ch);
}


void da9062_init(void)
{
	unsigned char value;
	da9062_i2c_ch = I2C_CH_MASTER0;
	da9062_initialized = 1;

	value = da9062_read(0x0);

	da9062_write(DA9062AA_BUCK1_CFG, 0x80);
	da9062_write(DA9062AA_BUCK2_CFG, 0x80);
	da9062_write(DA9062AA_BUCK3_CFG, 0x80);
	da9062_write(DA9062AA_BUCK4_CFG, 0x80);
	dprintf(INFO, "CFG1:0x%x, CFG2:0x%x, CFG3:0x%x, CFG4:0x%x\n",
			da9062_read(DA9062AA_BUCK1_CFG), da9062_read(DA9062AA_BUCK2_CFG),
			da9062_read(DA9062AA_BUCK3_CFG), da9062_read(DA9062AA_BUCK4_CFG));

	dprintf(INFO, "%s: id:0x%x\n", __func__, value);
}

int da9062_output_control(da9062_src_id id, int onoff)
{
	unsigned char reg, old_value, value, mask;

	switch (id){
	case DA9062_ID_BUCK1:
		reg = DA9062AA_BUCK1_CFG;
		mask = DA9062AA_BUCK1_PD_DIS_MASK;
		break;
	case DA9062_ID_BUCK2:
		reg = DA9062AA_BUCK2_CFG;
		mask = DA9062AA_BUCK2_PD_DIS_MASK;
		break;
	case DA9062_ID_BUCK3:
		reg = DA9062AA_BUCK3_CFG;
		mask = DA9062AA_BUCK3_PD_DIS_MASK;
		break;
	case DA9062_ID_BUCK4:
		reg = DA9062AA_BUCK4_CFG;
		mask = DA9062AA_BUCK4_PD_DIS_MASK;
		break;
	case DA9062_ID_LDO1:
		break;
	default:
		return -1;
	}

	old_value = da9062_read(reg);
	if (onoff)
		value = (old_value | mask);
	else
		value = (old_value & ~mask);

	da9062_write(reg, value);

	return 0;
}

static unsigned int da9062_buck_get_voltage(da9062_src_id id)
{
    unsigned char reg = 0;
    unsigned int value = 0, offset = 0, mask = 0, step = 0;

    switch(id) {
        case DA9062_ID_BUCK1:
        reg = DA9062AA_VBUCK1_A;
        offset = 300;
        step = 10;
        mask = DA9062AA_VBUCK1_A_MASK;
        break;
        case DA9062_ID_BUCK2:
        reg = DA9062AA_VBUCK2_A;
        offset = 300;
        step = 10;
        mask = DA9062AA_VBUCK2_A_MASK;
        break;
        case DA9062_ID_BUCK3:
        reg = DA9062AA_VBUCK3_A;
        offset = 800;
        step = 20;
        mask = DA9062AA_VBUCK3_A_MASK;
        break;
        case DA9062_ID_BUCK4:
        reg = DA9062AA_VBUCK4_A;
        offset = 530;
        step = 10;
        mask = DA9062AA_VBUCK4_A_MASK;
        break;
        case DA9062_ID_LDO1:
        break;
        default:
        return -1;
        break;
    }

    value = da9062_read(reg);
    value &= mask;
    value = offset + step*value;

    return value;
}

static int da9062_buck_set_voltage(da9062_src_id id, unsigned int mV)
{
	unsigned char reg = 0;
    unsigned int value = 0, pvalue = 0;
	unsigned char step = 0, mask = 0;
	unsigned int max = 0, min = 0;

	switch (id) {
		case DA9062_ID_BUCK1:
			reg = DA9062AA_VBUCK1_A;
			max = 1570; 
            min = 300;
            step = 10; 
            mask = DA9062AA_VBUCK1_A_MASK;
			break;
		case DA9062_ID_BUCK2:
			reg = DA9062AA_VBUCK2_A;
            max = 1570; 
            min = 300;
            step = 10; 
            mask = DA9062AA_VBUCK2_A_MASK;
            break;
        case DA9062_ID_BUCK3:
            reg = DA9062AA_VBUCK3_A;
            max = 3340; 
            min = 800;
            step = 20; 
            mask = DA9062AA_VBUCK3_A_MASK;
            break;
       case DA9062_ID_BUCK4:
           reg = DA9062AA_VBUCK4_A;
           max = 1800; 
           min = 530;
           step = 10; 
           mask = DA9062AA_VBUCK4_A_MASK;
           break; 
		default:
			return -1;
	}

	if (mV > max || mV < min){
		dprintf(INFO,"denied DA9062_BUCK%d value set!\n", id);
		return -1;
	}

	pvalue = da9062_read(reg);
    pvalue &= ~mask;
	value = (mV - min) / step;
	value = value | pvalue;

	dprintf(INFO, "%s: id:%d, mV:%d, reg:0x%x, value:0x%x\n", __func__, id, mV, reg, value);
	da9062_write(reg, value);
	da9062_output_control(id, true);
	return 0;
}

int choose_source_type(int type)
{
    switch(type)
    {
        case PWR_CPU0:
        return DA9062_ID_BUCK1;
        break;
        case PWR_GV:
        return DA9062_ID_BUCK2;
        break;
        case PWR_CORE:
        return DA9062_ID_BUCK3;
        break;
        case PWR_CPU1:
        return DA9062_ID_BUCK4;
        break;
        case PWR_IO:
        return DA9062_ID_LDO1;
        break;
        default:
        return -1;
        break;
    }
}

unsigned int da9062_get_voltage(int type)
{
    unsigned int vol;
    da9062_src_id id;

    id = choose_source_type(type);

    vol = da9062_buck_get_voltage(id);

    return vol;
}

int da9062_set_voltage(int type, unsigned int mV)
{
    da9062_src_id id;

	if (da9062_initialized == 0)
		return -1;

    id = choose_source_type(type);

	/* power off */
	if (mV == 0)
		da9062_output_control(id, false);
	else {
		switch (id){
			case DA9062_ID_BUCK1:
			case DA9062_ID_BUCK2:
			case DA9062_ID_BUCK3:
            case DA9062_ID_BUCK4:
				da9062_buck_set_voltage(id, mV);
				break;
			case DA9062_ID_LDO1:			
//				da9062_ldo_set_voltage(id, mV);
				break;
			default:
				return -1;
		}
	}

	return 0;
}

int da9062_set_power(int type, int onoff)
{
    da9062_src_id id;

	if (da9062_initialized == 0)
		return -1;

    id = choose_source_type(type);

	if (onoff)
		da9062_output_control(id, true);
	else
		da9062_output_control(id, false);

	return 0;
}
#endif


