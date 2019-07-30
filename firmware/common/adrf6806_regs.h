/* -*- mode: c -*- */

#ifndef __ADRF6806_REGS_DEF
#define __ADRF6806_REGS_DEF

/* Generate static inline accessors that operate on the global
 * regs. Done this way to (1) allow defs to be scraped out and used
 * elsewhere, e.g. in scripts, (2) to avoid dealing with endian
 * (structs). This may be used in firmware, or on host predefined
 * register loads. */

#define ADRF6806_REG_SET_CLEAN(_d, _r) (_d->regs_dirty &= ~(1UL<<_r))
#define ADRF6806_REG_SET_DIRTY(_d, _r) (_d->regs_dirty |= (1UL<<_r))

/* On set_, register is always set dirty, even if nothing
 * changed. This makes sure that write that have side effects,
 * e.g. frequency setting, are not skipped. */

/* n=name, r=regnum, o=offset (bits from LSB), l=length (bits) */
#define __MREG__(n,r,o,l) \
static inline uint32_t get_##n(adrf6806_driver_t* const _d) { \
	return (_d->regs[r] >> (o-l+1)) & ((1<<l)-1);	\
} \
static inline void set_##n(adrf6806_driver_t* const _d, uint16_t v) {      \
	_d->regs[r] &= ~(((1<<l)-1)<<(o-l+1));  \
	_d->regs[r] |= ((v&((1<<l)-1))<<(o-l+1)); \
	ADRF6806_REG_SET_DIRTY(_d, r); \
}

/* REG 0 */
__MREG__(ADRF6806_DIV_MODE,     0,10,1)
__MREG__(ADRF6806_INT_DIV,      0,9,7)
/* REG 1 */
__MREG__(ADRF6806_MOD,          1,13,11)
/* REG 2 */
__MREG__(ADRF6806_FRAC,         2,13,11)
/* REG 3 */
__MREG__(ADRF6806_DITH_MAG,     3,22,2)
#define ADRF6806_DITH_MAG_1		3
#define ADRF6806_DITH_MAG_3		2
#define ADRF6806_DITH_MAG_7		1
#define ADRF6806_DITH_MAG_15	0
__MREG__(ADRF6806_DITH_EN,      3,20,1)
__MREG__(ADRF6806_DITH_RESTART, 3,19,17)
/* REG 4 */
__MREG__(ADRF6806_REF_MUX_SEL,  4,23,3)
#define ADRF6806_MUX_SOURCE_LOCKDETECT	0
#define ADRF6806_MUX_SOURCE_VPTAT	1
#define ADRF6806_MUX_SOURCE_1XREFIN	2
#define ADRF6806_MUX_SOURCE_05XREFIN	3
#define ADRF6806_MUX_SOURCE_2XREFIN	4
#define ADRF6806_MUX_SOURCE_TRISTATE	5

__MREG__(ADRF6806_INPUT_REF,    4,20,2)
#define ADRF6806_INPUT_2XREFIN	0
#define ADRF6806_INPUT_1XREFIN	1
#define ADRF6806_INPUT_05XREFIN	2
#define ADRF6806_INPUT_025XREFIN	3
__MREG__(ADRF6806_CP_REF,       4,18,1)
#define ADRF6806_CP_REF_INTERN	0
#define ADRF6806_CP_REF_EXTERN	1
__MREG__(ADRF6806_PFD_POL,      4,17,1)
#define ADRF6806_PFD_POL_NEG	0
#define ADRF6806_PFD_POL_POS	1
__MREG__(ADRF6806_PFD_PH_OFFS,  4,16,5)
__MREG__(ADRF6806_CP_CURRMUL,   4,11,2)
#define ADRF6806_CP_CURRMUL_1	0
#define ADRF6806_CP_CURRMUL_2	1
#define ADRF6806_CP_CURRMUL_3	2
#define ADRF6806_CP_CURRMUL_4	3
__MREG__(ADRF6806_CP_CTL_SRC,   4,9,1)
#define ADRF6806_CP_CTLSRC_REG	0
#define ADRF6806_CP_CTLSRC_PFD	1
__MREG__(ADRF6806_CP_CTL,       4,8,2)
#define ADRF6806_CP_CTL_BOTH	0
#define ADRF6806_CP_CTL_DOWN	1
#define ADRF6806_CP_CTL_UP	2
#define ADRF6806_CP_CTL_TRISTATE	3
__MREG__(ADRF6806_DIV_EDGE_SENS,4,5,1)
__MREG__(ADRF6806_REF_EDGE_SENS,4,4,1)
#define ADRF6806_EDGE_FALLING	0
#define ADRF6806_EDGE_RISING	1
__MREG__(ADRF6806_PFD_ABD,      4,4,2)
#define ADRF6806_ABDLY_0NS		0
#define ADRF6806_ABDLY_05NS		1
#define ADRF6806_ABDLY_075NS	2
#define ADRF6806_ABDLY_09NS		3

/* REG 5 */
__MREG__(ADRF6806_DEMOD_BIAS_EN,5,7,1)
__MREG__(ADRF6806_LP_EN,        5,5,1)
__MREG__(ADRF6806_LO_INOUT_CTL, 5,4,1)
#define ADRF6806_LO_INOUT_OUT	0
#define ADRF6806_LO_INOUT_IN	1
__MREG__(ADRF6806_LO_DRV_EN,    5,3,1)

/* REG 6 */
__MREG__(ADRF6806_CP_EN,        6,20,1)
__MREG__(ADRF6806_L3_EN,        6,19,1)
__MREG__(ADRF6806_LV_EN,        6,18,1)
__MREG__(ADRF6806_VCO_EN,       6,17,1)
__MREG__(ADRF6806_VCO_SW,       6,16,1)
#define ADRF6806_VCO_REGULAR	0
#define ADRF6806_VCO_BANDCAL	1
__MREG__(ADRF6806_VCO_AMPL,     6,15,6)
__MREG__(ADRF6806_VCO_BS_SRC,   6,9,1)
#define ADRF6806_VCO_BS_BANDCAL	0
#define ADRF6806_VCO_BS_SPI	1
__MREG__(ADRF6806_VCO_BANDSEL,  6,8,6)

/* REG 7 */
__MREG__(ADRF6806_DIV_AB,       7,9,2)
#define ADRF6806_DIV_AB_2		0
#define ADRF6806_DIV_AB_3		1
#define ADRF6806_DIV_AB_4		2
#define ADRF6806_DIV_AB_5		3
__MREG__(ADRF6806_DIV_SEL,      7,7,2)
#define ADRF6806_DIV_SEL_B		0
#define ADRF6806_DIV_SEL_2		1
#define ADRF6806_DIV_SEL_4		2
#define ADRF6806_DIV_SEL_8		3
__MREG__(ADRF6806_ODIV_CTL,     7,5,2)
#define ADRF6806_ODIV_4			0
#define ADRF6806_ODIV_6			2
#define ADRF6806_ODIV_8			3


#endif // __ADRF6806_REGS_DEF
