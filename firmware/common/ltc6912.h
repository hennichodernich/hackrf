/*
 * Copyright 2012 Will Code? (TODO: Proper attribution)
 * Copyright 2014 Jared Boone <jared@sharebrained.com>
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

#ifndef __LTC6912_H
#define __LTC6912_H

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "spi_bus.h"

typedef enum {
	LTC6912_MODE_SHUTDOWN,
	LTC6912_MODE_ACTIVE
} ltc6912_mode_t;

struct ltc6912_driver_t;
typedef struct ltc6912_driver_t ltc6912_driver_t;

struct ltc6912_driver_t {
	spi_bus_t* const bus;
	void (*target_init)(ltc6912_driver_t* const drv);
	void (*set_mode)(ltc6912_driver_t* const drv, const ltc6912_mode_t new_mode);
	ltc6912_mode_t mode;
};

/* Initialize chip. */
extern void ltc6912_setup(ltc6912_driver_t* const drv);

extern void ltc6912_write(ltc6912_driver_t* const drv, uint8_t v);

ltc6912_mode_t ltc6912_mode(ltc6912_driver_t* const drv);
void ltc6912_set_mode(ltc6912_driver_t* const drv, const ltc6912_mode_t new_mode);

bool ltc6912_set_bb_gain(ltc6912_driver_t* const drv, const uint32_t gain_db);

#endif // __LTC6912_H
