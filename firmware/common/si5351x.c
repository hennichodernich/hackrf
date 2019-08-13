/*
 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "si5351x.h"

enum pll_sources active_clock_source;

/* write to single register */
void si5351x_write_single(si5351x_driver_t* const drv, uint8_t reg, uint8_t val)
{
	const uint8_t data_tx[] = { reg, val };
	si5351x_write(drv, data_tx, 2);
}

/* read single register */
uint8_t si5351x_read_single(si5351x_driver_t* const drv, uint8_t reg)
{
	const uint8_t data_tx[] = { reg };
	uint8_t data_rx[] = { 0x00 };
	i2c_bus_transfer(drv->bus, drv->i2c_address, data_tx, 1, data_rx, 1);
	return data_rx[0];
}

/*
 * Write to one or more contiguous registers. data[0] should be the first
 * register number, one or more values follow.
 */
void si5351x_write(si5351x_driver_t* const drv, const uint8_t* const data, const size_t data_count)
{
	i2c_bus_transfer(drv->bus, drv->i2c_address, data, data_count, NULL, 0);
}

/* Disable all CLKx outputs. */
void si5351x_disable_all_outputs(si5351x_driver_t* const drv)
{
	uint8_t data[] = { 3, 0xFF };
	si5351x_write(drv, data, sizeof(data));
}

/* Turn off OEB pin control for all CLKx */
void si5351x_disable_oeb_pin_control(si5351x_driver_t* const drv)
{
	uint8_t data[] = { 9, 0xFF };
	si5351x_write(drv, data, sizeof(data));
}

/* Power down all CLKx */
void si5351x_power_down_all_clocks(si5351x_driver_t* const drv)
{
#ifdef HNCH
	uint8_t data[] = { 16
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN 
	, SI5351X_CLK_POWERDOWN
	};
#else
	uint8_t data[] = { 16
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN
	, SI5351X_CLK_POWERDOWN | SI5351X_CLK_INT_MODE 
	, SI5351X_CLK_POWERDOWN | SI5351X_CLK_INT_MODE
	};
#endif
	si5351x_write(drv, data, sizeof(data));
}

/*
 * Register 183: Crystal Internal Load Capacitance
 * Reads as 0xE4 on power-up
 * Set to 8pF based on crystal specs and HackRF One testing
 */
void si5351x_set_crystal_configuration(si5351x_driver_t* const drv)
{
	uint8_t data[] = { 183, 0x80 };
	si5351x_write(drv, data, sizeof(data));
}

/*
 * Register 187: Fanout Enable
 * Turn on XO and MultiSynth fanout only.
 */
void si5351x_enable_xo_and_ms_fanout(si5351x_driver_t* const drv)
{
	uint8_t data[] = { 187, 0xD0 };
	si5351x_write(drv, data, sizeof(data));
}

/*
 * Register 15: PLL Input Source
 * CLKIN_DIV=0 (Divide by 1)
 * PLLA_SRC=0 (XTAL)
 * PLLB_SRC=1 (CLKIN)
 */
void si5351x_configure_pll_sources(si5351x_driver_t* const drv)
{
	uint8_t data[] = { 15, 0x08 };

	si5351x_write(drv, data, sizeof(data));
}

/* MultiSynth NA (PLLA) and NB (PLLB) */
void si5351x_configure_pll_multisynth(si5351x_driver_t* const drv)
{
	/*PLLA: 25MHz XTAL * (0x0e00+512)/128 = 800mhz -> int mode */
	uint8_t data[] = { 26, 0x00, 0x01, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00 };
	si5351x_write(drv, data, sizeof(data));
#ifndef HNCH
	/*PLLB: 10MHz CLKIN * (0x2600+512)/128 = 800mhz */
	data[0] = 34;
	data[4] = 0x26;
	si5351x_write(drv, data, sizeof(data));
#endif
}

void si5351x_reset_pll(si5351x_driver_t* const drv)
{
	/* reset PLLA and PLLB */
	uint8_t data[] = { 177, 0xA0 };
	si5351x_write(drv, data, sizeof(data));
}

void si5351x_configure_multisynth(si5351x_driver_t* const drv,
		const uint_fast8_t ms_number,
		const uint32_t p1, const uint32_t p2, const uint32_t p3,
    	const uint_fast8_t r_div)
{
	/*
	 * TODO: Check for p3 > 0? 0 has no meaning in fractional mode?
	 * And it makes for more jitter in integer mode.
	 */
	/*
	 * r is the r divider value encoded:
	 *   0 means divide by 1
	 *   1 means divide by 2
	 *   2 means divide by 4
	 *   ...
	 *   7 means divide by 128
	 */
	const uint_fast8_t register_number = 42 + (ms_number * 8);
	uint8_t data[] = {
			register_number,
			(p3 >> 8) & 0xFF,
			(p3 >> 0) & 0xFF,
			(r_div << 4) | (0 << 2) | ((p1 >> 16) & 0x3),
			(p1 >> 8) & 0xFF,
			(p1 >> 0) & 0xFF,
			(((p3 >> 16) & 0xF) << 4) | (((p2 >> 16) & 0xF) << 0),
			(p2 >> 8) & 0xFF,
			(p2 >> 0) & 0xFF };
	si5351x_write(drv, data, sizeof(data));
}

void si5351x_configure_clock_control(si5351x_driver_t* const drv, const enum pll_sources source)
{
	uint8_t pll;
#if (defined RAD1O || defined HNCH)
	(void) source;
	/* PLLA on XTAL */
	pll = SI5351X_CLK_PLL_SRC_A;
#endif

#if (defined JAWBREAKER || defined HACKRF_ONE)
	if (source == PLL_SOURCE_CLKIN) {
		/* PLLB on CLKIN */
		pll = SI5351X_CLK_PLL_SRC_B;
	} else {
		/* PLLA on XTAL */
		pll = SI5351X_CLK_PLL_SRC_A;
	}
#endif

#ifdef HNCH
	uint8_t data[] = {16
	,SI5351X_CLK_FRAC_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_SELF) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_8MA)
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_0_4) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_8MA)
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_0_4) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_8MA) | SI5351X_CLK_INV
	 };
#else
	/* Clock to CPU is deactivated as it is not used and creates noise */
	/* External clock output is deactivated as it is not used and creates noise */
	uint8_t data[] = {16
	,SI5351X_CLK_FRAC_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_SELF) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_2MA)
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_0_4) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_2MA)
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_0_4) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_2MA)
	,SI5351X_CLK_POWERDOWN | SI5351X_CLK_INT_MODE /*not connected, but: plla int mode*/
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_SELF) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_6MA) | SI5351X_CLK_INV
	,SI5351X_CLK_INT_MODE | SI5351X_CLK_PLL_SRC(pll) | SI5351X_CLK_SRC(SI5351X_CLK_SRC_MULTISYNTH_SELF) | SI5351X_CLK_IDRV(SI5351X_CLK_IDRV_4MA)
	,SI5351X_CLK_POWERDOWN | SI5351X_CLK_INT_MODE /*not connected, but: plla int mode*/
	,SI5351X_CLK_POWERDOWN | SI5351X_CLK_INT_MODE /*not connected, but: plla int mode*/
	 };
#endif
	si5351x_write(drv, data, sizeof(data));
}

void si5351x_enable_clock_outputs(si5351x_driver_t* const drv)
{
	/* Enable CLK outputs 0, 1, 2, 4, 5 only. */
	/* 7: Clock to CPU is deactivated as it is not used and creates noise */
	/* 3: External clock output is deactivated by default */
#ifdef HNCH
	uint8_t data[] = { 3, ~((1 << 0) | (1 << 1) | (1 << 2))};
#else
	uint8_t data[] = { 3, ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 4) | (1 << 5))};
#endif
	si5351x_write(drv, data, sizeof(data));
}

void si5351x_set_int_mode(si5351x_driver_t* const drv, const uint_fast8_t ms_number, const uint_fast8_t on){
  uint8_t data[] = {16, 0};

  if(ms_number < 8){
      data[0] = 16 + ms_number;
      data[1] = si5351x_read_single(drv, data[0]);

      if(on)
          data[1] |= SI5351X_CLK_INT_MODE;
      else
          data[1] &= ~(SI5351X_CLK_INT_MODE);

      si5351x_write(drv, data, 2);
  }
}

void si5351x_set_clock_source(si5351x_driver_t* const drv, const enum pll_sources source)
{
	si5351x_configure_clock_control(drv, source);
	active_clock_source = source;
}

void si5351x_activate_best_clock_source(si5351x_driver_t* const drv)
{
#ifndef HNCH
	uint8_t device_status = si5351x_read_single(drv, 0);

	if (device_status & SI5351X_LOS) {
		/* CLKIN not detected */
		if (active_clock_source == PLL_SOURCE_CLKIN) {
			si5351x_set_clock_source(drv, PLL_SOURCE_XTAL);
		}
	} else {
		/* CLKIN detected */
		if (active_clock_source == PLL_SOURCE_XTAL) {
			si5351x_set_clock_source(drv, PLL_SOURCE_CLKIN);
		}
	}
#else
	si5351x_set_clock_source(drv, PLL_SOURCE_XTAL);
#endif
}
