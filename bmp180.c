#include <inttypes.h>
#include <twi/twi.h>
#include <util/twi.h>
#include <util/delay.h>
#include "bmp180.h"

struct bmp180_callibration callibration = {
	.ac1 = 408,
	.ac2 = -72,
	.ac3 = -14383,
	.ac4 = 32741,
	.ac5 = 32757,
	.ac6 = 23153,
	.b1 = 6190,
	.b2 = 4,
	.mb = -32768,
	.mc = -8711,
	.md = 2868,
};

struct bmp180_measurements measurements = {
	.ut = 27898,
	.up = 23843,
};

uint16_t bmp180_read_memory(uint8_t address)
{
	twi_start();
	twi_write(BMP180_ADDRESS | TW_WRITE);
	twi_write(address);
	twi_start();
	twi_write(BMP180_ADDRESS | TW_READ);
	uint8_t msb = twi_read_ack();
	uint8_t lsb = twi_read_nack();
	return msb << 8 | lsb;
}

void bmp180_write_memory(uint8_t address, uint8_t value)
{
	twi_start();
	twi_write(BMP180_ADDRESS | TW_WRITE);
	twi_write(address);
	twi_write(value);
}

void bmp180_get_callibration_params()
{	
	callibration.ac1 = (int16_t) bmp180_read_memory(0xAA);
	callibration.ac2 = (int16_t) bmp180_read_memory(0xAC);
	callibration.ac3 = (int16_t) bmp180_read_memory(0xAE);
	callibration.ac4 = bmp180_read_memory(0xB0);
	callibration.ac5 = bmp180_read_memory(0xB2);
	callibration.ac6 = bmp180_read_memory(0xB4);
	callibration.b1  = (int16_t) bmp180_read_memory(0xB6);
	callibration.b2  = (int16_t) bmp180_read_memory(0xB8);
	callibration.mb  = (int16_t) bmp180_read_memory(0xBA);
	callibration.mc  = (int16_t) bmp180_read_memory(0xBC);
	callibration.md  = (int16_t) bmp180_read_memory(0xBE);
}

void bmp180_get_raw_temperature()
{
	bmp180_write_memory(0xF4, 0x2E);
	_delay_ms(5);
	measurements.ut = (int32_t) bmp180_read_memory(0xF6);
}

uint8_t bmp180_calculate_true_temperature(int32_t *temperature)
{
  	x1 = ((int32_t) measurements.ut - callibration.ac6) * callibration.ac5 >> 15;
  	x2 = ((int32_t) callibration.mc << 11) / (x1 + callibration.md);
  	b5 = x1 + x2;
 	t = (b5 + 8) >> 4;
 	*temperature = t;
	return 1;
}