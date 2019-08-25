#ifndef BMP180_H_
#define BMP180_H_

#define BMP180_ADDRESS 0xEE

struct bmp180_callibration {
	int16_t		ac1;
	int16_t		ac2;
	int16_t		ac3;
	uint16_t	ac4;
	uint16_t	ac5;
	uint16_t	ac6;
	int16_t		b1;
	int16_t		b2;
	int16_t		mb;
	int16_t		mc;
	int16_t		md;
};

struct bmp180_measurements {
	int32_t ut;
	int32_t up;
};

int32_t	x1, x2, b5, t;

void bmp180_read_memory(uint8_t address, uint8_t buffer[], uint8_t size);
void bmp180_write_memory(uint8_t address, uint8_t value);
void bmp180_get_callibration_params();
void bmp180_get_raw_temperature();
uint8_t bmp180_calculate_true_temperature(int32_t *temperature);


#endif /* BMP180_H_ */