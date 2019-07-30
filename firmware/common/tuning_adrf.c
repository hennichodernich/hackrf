/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
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
#include "tuning_adrf.h"

#include "hackrf-ui.h"
#include "adrf6806.h"

#include <hackrf_core.h>

#include <sgpio.h>

#define MIN_LO_FREQ_MHZ ( 35)
#define MAX_LO_FREQ_MHZ (525)

uint64_t freq_cache = 0;

bool set_freq(const uint64_t freq)
{
	bool success = true;

	const uint32_t freq_mhz = freq / 1000000;

	if((freq_mhz >= MIN_LO_FREQ_MHZ)&&(freq_mhz <= MAX_LO_FREQ_MHZ))
	{
		adrf6806_set_frequency(&adrf6806, (uint32_t)freq);
	}
/*	else
	{
		success = false;
	}
*/
	if( success ) {
		freq_cache = freq;
		hackrf_ui_setFrequency(freq);
		operacake_set_range(freq_mhz);
	}
	return success;
}

bool set_freq_explicit(const uint64_t if_freq_hz, const uint64_t lo_freq_hz,
		const rf_path_filter_t path)
{
	return false;
}
