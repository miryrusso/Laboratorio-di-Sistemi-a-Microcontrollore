/*
 * main.c
 *
 *  Created on: 06 ott 2023
 *      Author: miriana
 */

//Variabile che conta il tempo e che possiamo visualizzare sul display
#include "stm32_unict_lib.h"

int count = 0;

void setup(void)
{
	DISPLAY_init(); //inizializzazione display
	GPIO_init(GPIOB); //inizializzazione GPIOB
	GPIO_config_output(GPIOB, 0); //inizializzazione LED
	GPIO_config_input(GPIOB, 4); //inizializzazione pulsante da premere
	GPIO_write(GPIOB, 0, 0); //Led spento
}

void loop(void)
{
	//beccare l'evento cliccare pulsante
	int button_state = GPIO_read(GPIOB, 4); //leggi stato pulsante
	char s[5];

	if(button_state == 0){ //quando il pulsante è premuto == 0

		count = 10;

		GPIO_write(GPIOB, 0, 1);

			while(count > 0){
				sprintf(s, "%4d ", count);
				DISPLAY_puts(0,s);

				for(int i = 0; i<1000; i++){
					delay_ms(1);
					button_state = GPIO_read(GPIOB, 4);

					if(button_state == 0){ //se premessimo il pulsante riparte il timer da 10
						count = 11;
					}

				}



				count--;

			}

		sprintf(s, "%4d ", count); //Stampiamo sul display partendo da destra
		DISPLAY_puts(0,s);
		GPIO_write(GPIOB, 0, 0);
		//DISPLAY_puts(0,"SUCA");
	}
}

int main()
{
	setup();
	for(;;) {
		loop();
	}
	return 0;
}

