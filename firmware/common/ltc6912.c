#include <stdint.h>
#include <string.h>
#include "ltc6912.h"

/*
 * Set up pins for GPIO and SPI control, configure SSP peripheral for SPI, and
 * set our own default register configuration.
 */
void ltc6912_setup(ltc6912_driver_t* const drv)
{
	ltc6912_write(drv, 0x11);
}

void ltc6912_write(ltc6912_driver_t* const drv, uint8_t v) {

	ssp1_set_mode_ltc6912();
	spi_bus_transfer(drv->bus, &v, 1);
}


void ltc6912_set_mode(ltc6912_driver_t* const drv, const ltc6912_mode_t new_mode) {
	drv->set_mode(drv, new_mode);
}

ltc6912_mode_t ltc6912_mode(ltc6912_driver_t* const drv) {
	return drv->mode;
}

bool ltc6912_set_bb_gain(ltc6912_driver_t* const drv, const uint32_t gain_db) {
	uint8_t gainval = 0x00;	// gain disabled

	if (gain_db >= 40)
		gainval = 0x77;
	else if (gain_db >= 34)
		gainval = 0x66;
	else if (gain_db >= 26)
		gainval = 0x55;
	else if (gain_db >= 20)
		gainval = 0x44;
	else if (gain_db >= 14)
		gainval = 0x33;
	else if (gain_db >= 6)
		gainval = 0x22;
	else
		gainval = 0x11;

	ltc6912_write(drv, gainval);

	return true;
}
