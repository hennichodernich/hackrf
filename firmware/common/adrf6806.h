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

#ifndef __ADRF6806_H
#define __ADRF6806_H

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "spi_bus.h"

/* 32 registers, each containing 10 bits of data. */
#define ADRF6806_NUM_REGS 8
#define ADRF6806_PFD_FREQUENCY 25000000

typedef enum {
	ADRF6806_MODE_SHUTDOWN,
	ADRF6806_MODE_RX
} adrf6806_mode_t;

struct adrf6806_driver_t;
typedef struct adrf6806_driver_t adrf6806_driver_t;

struct adrf6806_driver_t {
	spi_bus_t* const bus;
	void (*target_init)(adrf6806_driver_t* const drv);
	void (*set_mode)(adrf6806_driver_t* const drv, const adrf6806_mode_t new_mode);
	adrf6806_mode_t mode;
	uint32_t regs[ADRF6806_NUM_REGS];
	uint8_t regs_dirty;
};

/* Initialize chip. */
extern void adrf6806_setup(adrf6806_driver_t* const drv);

/* Write value to register via SPI and save a copy to memory. Mark
 * clean. */
extern void adrf6806_reg_write(adrf6806_driver_t* const drv, uint8_t r, uint32_t v);

/* Write all dirty registers via SPI from memory. Mark all clean. Some
 * operations require registers to be written in a certain order. Use
 * provided routines for those operations. */
extern void adrf6806_regs_commit(adrf6806_driver_t* const drv);

adrf6806_mode_t adrf6806_mode(adrf6806_driver_t* const drv);
void adrf6806_set_mode(adrf6806_driver_t* const drv, const adrf6806_mode_t new_mode);

/* Set frequency in Hz. Frequency setting is a multi-step function
 * where order of register writes matters. */
extern void adrf6806_set_frequency(adrf6806_driver_t* const drv, uint32_t freq);
bool adrf6806_set_lna_gain(adrf6806_driver_t* const drv, const uint32_t gain_db);

void frap(double x, uint16_t maxden, uint16_t *num, uint16_t *den);


#endif // __ADRF6806_H
