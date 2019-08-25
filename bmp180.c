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

void bmp180_read_memory(uint8_t address, uint8_t buffer[], uint8_t size)
{
	twi_start();
	twi_write(BMP180_ADDRESS | TW_WRITE);
	twi_write(address);
	twi_start();
	twi_write(BMP180_ADDRESS | TW_READ);
	
	for (uint8_t i = 0; i < size; i++)
	{
		if (i == size - 1)
		{
			buffer[i] = twi_read_nack();
		}
		else
		{
			buffer[i] = twi_read_ack();
		}
	}
	
	twi_stop();
}

void bmp180_write_memory(uint8_t address, uint8_t value)
{
	twi_start();
	twi_write(BMP180_ADDRESS | TW_WRITE);
	twi_write(address);
	twi_write(value);
	twi_stop();
}

void bmp180_get_callibration_params()
{
	uint8_t buffer[2];
	
	bmp180_read_memory(0xAA, buffer, 2);
	callibration.ac1 = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xAC, buffer, 2);
	callibration.ac2 = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xAE, buffer, 2);
	callibration.ac3 = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xB0, buffer, 2);
	callibration.ac4 = buffer[0] << 8 | buffer[1];
	
	bmp180_read_memory(0xB2, buffer, 2);
	callibration.ac5 = buffer[0] << 8 | buffer[1];
	
	bmp180_read_memory(0xB4, buffer, 2);
	callibration.ac6 = buffer[0] << 8 | buffer[1];
	
	bmp180_read_memory(0xB6, buffer, 2);
	callibration.b1 = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xB8, buffer, 2);
	callibration.b2 = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xBA, buffer, 2);
	callibration.mb = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xBC, buffer, 2);
	callibration.mc = (int16_t) (buffer[0] << 8 | buffer[1]);
	
	bmp180_read_memory(0xBE, buffer, 2);
	callibration.md = (int16_t) (buffer[0] << 8 | buffer[1]);
}

void bmp180_get_raw_temperature()
{
	uint8_t buffer[2];
	bmp180_write_memory(0xF4, 0x2E);
	_delay_ms(5);
	bmp180_read_memory(0xF6, buffer, 2);
	measurements.ut = (int32_t) (buffer[0] << 8 | buffer[1]);
}

void bmp180_get_raw_pressure()
{
	uint8_t buffer[3];
	bmp180_write_memory(0xF4, 0x74);
	_delay_ms(8);
	bmp180_read_memory(0xF6, buffer, 3);
	measurements.up = (((int32_t) buffer[0] << 16 | (int32_t) buffer[1] << 8 | (int32_t) buffer[2]) >> 7);
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

uint8_t bmp180_calculate_true_pressure(int32_t *pressure)
{
	b6 = b5 - 4000;
	x1 = ((int32_t) callibration.b2 * (b6 * b6 >> 12)) >> 11;
	x2 = (int32_t) callibration.ac2 * b6 >> 11;
	x3 = x1 + x2;
	b3 = ((((int32_t) callibration.ac1 * 4 + x3) << 1) + 2) >> 2;
	x1 = (int32_t) callibration.ac3 * b6 >> 13;
	x2 = ((int32_t) callibration.b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (uint32_t) callibration.ac4 * (uint32_t) (x3 + 32768) >> 15;
	b7 = ((uint32_t) measurements.up - b3) * (50000 >> 1);
	p  = (b7 < 0x80000000) ? (b7 << 1) / b4 : (b7 / b4) << 1;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p = p + ((x1 + x2 + 3791) >> 4);
	
	*pressure = p;
	return 1;
}