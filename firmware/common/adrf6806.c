#include <stdint.h>
#include <string.h>
#include "adrf6806.h"
#include "adrf6806_regs.h" // private register def macros

#define ABS(x) ((x)>0?(x):-(x))

/* Default register values. */
static const uint32_t adrf6806_regs_default[ADRF6806_NUM_REGS] = {
		0x000005c0,   /* 0 */
		0x00000000,   /* 1 */
		0x00000000,   /* 2 */
		0x00600008,   /* 3 */
		0x000aa780,   /* 4 */
		0x00000088,   /* 5 */
		0x001edd00,   /* 6 */
		0x00000050    /* 7 */
};

/* Set up all registers according to defaults specified in docs. */
static void adrf6806_init(adrf6806_driver_t* const drv)
{
	drv->target_init(drv);
	//adrf6806_set_mode(drv, ADRF6806_MODE_SHUTDOWN);

	memcpy(drv->regs, adrf6806_regs_default, sizeof(drv->regs));
	drv->regs_dirty = 0xff;

	/* Write default register values to chip. */
	adrf6806_regs_commit(drv);
}

/*
 * Set up pins for GPIO and SPI control, configure SSP peripheral for SPI, and
 * set our own default register configuration.
 */
void adrf6806_setup(adrf6806_driver_t* const drv)
{
	adrf6806_init(drv);

	set_ADRF6806_DITH_MAG(drv, ADRF6806_DITH_MAG_1);
	set_ADRF6806_DITH_EN(drv, 0);
	set_ADRF6806_DITH_RESTART(drv, 1);

	set_ADRF6806_REF_MUX_SEL(drv, ADRF6806_MUX_SOURCE_LOCKDETECT);
	set_ADRF6806_INPUT_REF(drv, ADRF6806_INPUT_1XREFIN);
	set_ADRF6806_CP_REF(drv, ADRF6806_CP_REF_INTERN);
	set_ADRF6806_PFD_POL(drv, ADRF6806_PFD_POL_POS);
	set_ADRF6806_PFD_PH_OFFS(drv, 10);	// 10*22.5°
	set_ADRF6806_CP_CURRMUL(drv, ADRF6806_CP_CURRMUL_2);
	set_ADRF6806_CP_CTL_SRC(drv, ADRF6806_CP_CTLSRC_PFD);
	set_ADRF6806_CP_CTL(drv, ADRF6806_CP_CTL_TRISTATE);
	set_ADRF6806_DIV_EDGE_SENS(drv, ADRF6806_EDGE_FALLING);
	set_ADRF6806_REF_EDGE_SENS(drv, ADRF6806_EDGE_FALLING);
	set_ADRF6806_PFD_ABD(drv, ADRF6806_ABDLY_0NS);

	set_ADRF6806_DEMOD_BIAS_EN(drv, 1);
	set_ADRF6806_LP_EN(drv, 0);
	set_ADRF6806_LO_INOUT_CTL(drv, ADRF6806_LO_INOUT_OUT);
	set_ADRF6806_LO_DRV_EN(drv, 0);

	set_ADRF6806_CP_EN(drv, 1);
	set_ADRF6806_L3_EN(drv, 1);
	set_ADRF6806_LV_EN(drv, 1);
	set_ADRF6806_VCO_EN(drv, 1);
	set_ADRF6806_VCO_SW(drv, ADRF6806_VCO_REGULAR);
	set_ADRF6806_VCO_AMPL(drv, 55);
	set_ADRF6806_VCO_BS_SRC(drv, ADRF6806_VCO_BS_BANDCAL);
	set_ADRF6806_VCO_BANDSEL(drv, 32);

	set_ADRF6806_DIV_AB(drv, ADRF6806_DIV_AB_2);
	set_ADRF6806_DIV_SEL(drv, ADRF6806_DIV_SEL_2);
	set_ADRF6806_ODIV_CTL(drv, ADRF6806_ODIV_4);

	adrf6806_regs_commit(drv);
}

static void adrf6806_write(adrf6806_driver_t* const drv, uint8_t r, uint32_t v) {
	uint32_t value = (r & 0x07) | (v & 0x00fffff8);
	uint8_t byte_array[3];
	int kk;

	for(kk=0;kk<3;kk++)
	{
		byte_array[kk]=(uint8_t)( (value >> ((2-kk)*8) ) & 0xff);
	}
	spi_bus_transfer(drv->bus, byte_array, 3);
}

void adrf6806_reg_write(adrf6806_driver_t* const drv, uint8_t r, uint32_t v)
{
	drv->regs[r] = v;
	adrf6806_write(drv, r, v);
	ADRF6806_REG_SET_CLEAN(drv, r);
}

static inline void adrf6806_reg_commit(adrf6806_driver_t* const drv, uint8_t r)
{
	adrf6806_reg_write(drv, r, drv->regs[r]);
}

void adrf6806_regs_commit(adrf6806_driver_t* const drv)
{
	int r;
	for(r = 0; r < ADRF6806_NUM_REGS; r++) {
		if ((drv->regs_dirty >> r) & 0x1) {
			adrf6806_reg_commit(drv, r);
		}
	}
}

void adrf6806_set_mode(adrf6806_driver_t* const drv, const adrf6806_mode_t new_mode) {
	drv->set_mode(drv, new_mode);
}

adrf6806_mode_t adrf6806_mode(adrf6806_driver_t* const drv) {
	return drv->mode;
}


void adrf6806_set_frequency(adrf6806_driver_t* const drv, uint32_t freq)
{

	uint8_t divab,divsel,lo_divider,int_part;
	uint16_t frac, mod;
	double ideal_divider, dec_part;

	if ((freq >= 35000000) && (freq < 43750000))
	{
		divab = ADRF6806_DIV_AB_5;
		divsel = ADRF6806_DIV_SEL_8;
	}
	if ((freq >= 43750000) && (freq < 58333333))
	{
		divab = ADRF6806_DIV_AB_4;
		divsel = ADRF6806_DIV_SEL_8;
	}
	if ((freq >= 58333333) && (freq < 70000000))
	{
		divab = ADRF6806_DIV_AB_3;
		divsel = ADRF6806_DIV_SEL_8;
	}
	if ((freq >= 70000000) && (freq < 87500000))
	{
		divab = ADRF6806_DIV_AB_5;
		divsel = ADRF6806_DIV_SEL_4;
	}
	if ((freq >= 87500000) && (freq < 116666666))
	{
		divab = ADRF6806_DIV_AB_4;
		divsel = ADRF6806_DIV_SEL_4;
	}
	if ((freq >= 116666666) && (freq < 140000000))
	{
		divab = ADRF6806_DIV_AB_3;
		divsel = ADRF6806_DIV_SEL_4;
	}
	if ((freq >= 140000000) && (freq < 175000000))
	{
		divab = ADRF6806_DIV_AB_5;
		divsel = ADRF6806_DIV_SEL_2;
	}
	if ((freq >= 175000000) && (freq < 233333333))
	{
		divab = ADRF6806_DIV_AB_4;
		divsel = ADRF6806_DIV_SEL_2;
	}
	if ((freq >= 233333333) && (freq < 350000000))
	{
		divab = ADRF6806_DIV_AB_3;
		divsel = ADRF6806_DIV_SEL_2;
	}
	if ((freq >= 350000000) && (freq <= 525000000))
	{
		divab = ADRF6806_DIV_AB_2;
		divsel = ADRF6806_DIV_SEL_2;
	}

	lo_divider = (divab + 2) * (1 << divsel) * 2;
    
    set_ADRF6806_DIV_AB(drv,divab);
    set_ADRF6806_DIV_SEL(drv,divsel);
    
    
	ideal_divider = (double)(freq * lo_divider) / 2.0 / ADRF6806_PFD_FREQUENCY;

	int_part = (uint8_t)ideal_divider;
	dec_part = ideal_divider - (double)int_part;

	frap(dec_part,2047,&frac,&mod);

	if (frac==0)
	{
		set_ADRF6806_DIV_MODE(drv,1);
		set_ADRF6806_INT_DIV(drv, int_part);
		set_ADRF6806_FRAC(drv,0);
		set_ADRF6806_MOD(drv,0);
	}
	else
	{
		while (mod < 40)
		{
			mod  *= 2;
			frac *= 2;
		}
		set_ADRF6806_DIV_MODE(drv,0);
		set_ADRF6806_INT_DIV(drv, int_part);
		set_ADRF6806_FRAC(drv,frac);
		set_ADRF6806_MOD(drv,mod);
	}

	adrf6806_regs_commit(drv);
}


bool adrf6806_set_lna_gain(adrf6806_driver_t* const drv, const uint32_t gain_db) {

	return true;
}

void frap(double x, uint16_t maxden, uint16_t *num, uint16_t *den)
{
	uint32_t m[2][2];
	double startx, error;
	uint32_t ai;

	startx = x;

	/* initialize matrix */
	m[0][0] = m[1][1] = 1;
	m[0][1] = m[1][0] = 0;

	/* loop finding terms until denom gets too big */
	while (m[1][0] *  ( ai = (uint32_t)x ) + m[1][1] <= maxden)
	{
		uint32_t t;
		t = m[0][0] * ai + m[0][1];
		m[0][1] = m[0][0];
		m[0][0] = t;
		t = m[1][0] * ai + m[1][1];
		m[1][1] = m[1][0];
		m[1][0] = t;
		if(x==(double)ai) break;     // AF: division by zero
		x = 1/(x - (double) ai);
		if(x>(double)0x7FFFFFFF) break;  // AF: representation failure
	}

	/* now remaining x is between 0 and 1/ai */
	/* approx as either 0 or 1/m where m is max that will fit in maxden */
	/* first try zero */
	*num = (uint16_t)(m[0][0]);
	*den = (uint16_t)(m[1][0]);

	//printf("%ld/%ld, error = %e\n", m[0][0], m[1][0], startx - ((double) m[0][0] / (double) m[1][0]));
	error = ABS(startx - ((double) m[0][0] / (double) m[1][0]));

	/* now try other possibility */
	ai = (maxden - m[1][1]) / m[1][0];
	m[0][0] = m[0][0] * ai + m[0][1];
	m[1][0] = m[1][0] * ai + m[1][1];
	//    printf("%ld/%ld, error = %e\n", m[0][0], m[1][0], startx - ((double) m[0][0] / (double) m[1][0]));
	if (ABS(startx - ((double) m[0][0] / (double) m[1][0])) < error)
	{
		*num = (uint16_t)(m[0][0]);
		*den = (uint16_t)(m[1][0]);
	}
}
