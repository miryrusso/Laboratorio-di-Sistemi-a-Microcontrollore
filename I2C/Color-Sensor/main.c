/*
 * main.c
 *
 *  Created on: Jan 10, 2024
 *      Author: Miriana
 */

#include "stm32_unict_lib.h"
#include <stdio.h>

#define TCS3472_ADDRESS (0x29<<1)
short who;

int main(void)
{
	CONSOLE_init();
	printf("Starting\n");
	delay_ms(500);
	I2C_init(I2C1, 100000);
	GPIO_PULL_UP(GPIOB, 8);
	GPIO_PULL_UP(GPIOB, 9);
	I2C_write_register(I2C1, TCS3472_ADDRESS, 0x80, 0x13); //Scriviamo il valore di 0x13 in 0x80
	delay_ms(500);

	I2C_read_register(I2C1, TCS3472_ADDRESS, 0x92, &who);
	printf("Who am i = %x\n", who);

	for (;;) {
		u_int16_t red_l, red_h, green_l, green_h, blue_h, blue_l, red, green, blue;

		I2C_write_register(I2C1, TCS3472_ADDRESS, 0x80, 0x03); //Scriviamo il valore di 0x13 in 0x80
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x96, &red_l);
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x97, &red_h);
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x98, &green_l);
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x99, &green_h);
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x9B, &blue_h);
		I2C_read_register(I2C1, TCS3472_ADDRESS, 0x9A, &blue_l);

		red = (red_h << 8) | red_l; //combinare i singoli byte
		green = (green_h << 8) | green_l; //combinare i singoli byte
		blue = (blue_h << 8) | blue_l; //combinare i singoli byte


		printf("Red %d, Green %d, Blue %d \n", red, green, blue);
		delay_ms(300);
		I2C_write_register(I2C1, TCS3472_ADDRESS, 0x80, 0x13); //Scriviamo il valore di 0x13 in 0x80
		delay_ms(200);

	}
}

