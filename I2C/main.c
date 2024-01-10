/*
 * imu.c
 *
 *  Created on: Nov 26, 2018
 *      Author: corrado
 */

#include "stm32_unict_lib.h"
#include <stdio.h>

short who, conf, ctrl;

#define HTS221_ADDRESS 0xbe

int main(void)
{
	CONSOLE_init();
	printf("Starting\n");
	delay_ms(500);
	I2C_init(I2C1, 100000);
	I2C_read_register(I2C1, HTS221_ADDRESS, 0xf, &who);
	printf("Who am i = %x\n", who);

	conf = 0b00110100;
	// 110 = Temp res 0.01 deg
	// 100 = Hum res 0.1 rH%
	I2C_write_register(I2C1, HTS221_ADDRESS, 0x10, conf);

	ctrl = 0b10000000;
	// PD = 1, activate sensor, one-shot mode
	I2C_write_register(I2C1, HTS221_ADDRESS, 0x20, ctrl);

	for (;;) {
		short temp_l, temp_h, temp, status = 0;
		ctrl = 1;
		// OneShot Enable (start sensing)mode
		I2C_write_register(I2C1, HTS221_ADDRESS, 0x21, ctrl);

		do {
			I2C_read_register(I2C1, HTS221_ADDRESS, 0x27, &status);
		} while ((status & 1) == 0);

		I2C_read_register(I2C1, HTS221_ADDRESS, 0x2a, &temp_l);
		I2C_read_register(I2C1, HTS221_ADDRESS, 0x2b, &temp_h);

		temp = (temp_h << 8) | temp_l;

		printf("Temp = %d\n", temp);
		delay_ms(500);
	}
}
