/*
 * codice.c
 *
 *  Created on: 16 ott 2023
 *      Author: miriana
 */
#include "stm32_unict_lib.h"

typedef enum{
	INIT,
	EROGAZIONE,
	STOP
}state;

int importo; //da visualizzare sul display al momento quando lo stato è impostato ad INIT
int quantita_carburante; //Quantità di carburante che corrisponde all'importo scelto
int carburante_erogato; //variabile che viene visualizzata sul display

state current_state;

char visualizza_importi[5];

void setup(void)
{
	DISPLAY_init();

	importo = 0;

	//pulsantini
	GPIO_init(GPIOB);
	//ledi giallo e verde
	GPIO_init(GPIOC);
	//10-> X
	GPIO_config_input(GPIOB,10);
	GPIO_config_EXTI(GPIOB, EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);
	//4->Y
	GPIO_config_input(GPIOB,4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);
	//5->Z
	GPIO_config_input(GPIOB,5);
	GPIO_config_EXTI(GPIOB, EXTI5);
	EXTI_enable(EXTI5, FALLING_EDGE);
	//6->T
	GPIO_config_input(GPIOB,6);
	GPIO_config_EXTI(GPIOB, EXTI6);
	EXTI_enable(EXTI6, FALLING_EDGE);

	//led rosso
	GPIO_config_output(GPIOB, 0);
	GPIO_write(GPIOB, 0, 0);

	//led giallo
	GPIO_config_output(GPIOC, 2);
	GPIO_write(GPIOC, 2, 0);

	DISPLAY_puts(0,"0");
	current_state = INIT;

	//decimal point
	DISPLAY_dp(1,0);

	carburante_erogato = 0;
	quantita_carburante = 0;

}

void loop(void)
{

	switch(current_state){
				case INIT:
					sprintf(visualizza_importi, "%4d", importo);
					DISPLAY_puts(0,visualizza_importi);
					break;

				case EROGAZIONE:
					for(int i = 0; carburante_erogato < quantita_carburante; i++){
						delay_ms(100);
						carburante_erogato += 5;
						if(i%5 == 0) //permettiamo al led di lampeggiare solo ogni 500ms
							GPIO_toggle(GPIOC, 2);
						//stampiamo il carburante erogato ogni 100ms
						sprintf(visualizza_importi, "%04d", carburante_erogato);
						DISPLAY_puts(0,visualizza_importi);
					}
					//spegniamo il led giallo
					GPIO_write(GPIOB, 2, 0);
					//Accendiamo il led rosso
					GPIO_write(GPIOB, 0, 1);
					//impostiamo lo stato automaticamente a STOP
					current_state = STOP;
					break;

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

//Interrupt sul pin 10, pulsante X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		if(current_state == INIT){
			importo += 20;
		}
		EXTI_clear(EXTI10);
	}
}

//Interrupt sul pin 4, pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		if(current_state == INIT){
			importo += 5;
		}
		EXTI_clear(EXTI4);
	}
}


void EXTI9_5_IRQHandler(void){
	//Interrupt sul pin 5, pulsante Z
	if(EXTI_isset(EXTI5)){
			if(current_state == INIT){
				importo = 0;
			}
			EXTI_clear(EXTI5);
		}
	//Interrupt sul pin 6, pulsante T
	if(EXTI_isset(EXTI6)){
			switch(current_state){
				case INIT:
					current_state = EROGAZIONE;
					quantita_carburante = importo * 150;
					DISPLAY_dp(1,1);
					break;
				case STOP:
					DISPLAY_puts(0,"0");
					current_state = INIT;
					setup();
					break;

			}

			EXTI_clear(EXTI6);
		}
}
