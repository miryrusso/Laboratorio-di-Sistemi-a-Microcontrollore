/*
 * codice.c
 * Version 2.0 with timer
 */

#include "stm32_unict_lib.h"

typedef enum{
	IDLE,
	INIT,
	EROGAZIONE,
	STOP
}state;

state current_state;
state last_state;

char s[5];
int count_time;
int importo;
int carburante_erogato ;
int quantita_carburante;
//costo del carburante  = 1.5 Euro al Litro
//Velocità di erogazione = 0.5 Litri


void setup(void)
{
	DISPLAY_init();
	importo = 0;
	carburante_erogato = 0;
	quantita_carburante = 0;
	count_time = 0;

	DISPLAY_puts(0, "0");
	DISPLAY_dp(1,0);


	//pulsantini e led rosso
	GPIO_init(GPIOB);

	//led giallo e verde
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

	//set state
	current_state = IDLE;
	last_state = IDLE;

	//Timer per segnalare le tempistiche
	TIM_init(TIM2);
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_config_timebase(TIM2, 8400, 5000); //interrupt ogni 500ms
	TIM_set(TIM2, 0);
	//TIM_off(TIM2);*

}

void loop(void)
{
	switch(current_state){
		case IDLE:
			setup();
			current_state = INIT;
			break;
		case INIT:

			//Visualizziamo importo

			sprintf(s, "%4d", importo);
			DISPLAY_puts(0, s);

			break;
		case EROGAZIONE:
			TIM_on(TIM2);

			for(int i = 0; carburante_erogato < quantita_carburante; i++){
				delay_ms(100); //serve perchè altrimenti è cosi veloce da non farci vedere come "scorre" il carburante
				carburante_erogato += 5;
				sprintf(s, "%4d", carburante_erogato);
				DISPLAY_puts(0,s);
			}
			TIM_off(TIM2);
			current_state = STOP;
			break;
		case STOP:

			GPIO_write(GPIOB, 0, 1);
			GPIO_write(GPIOC, 2, 0);
			break;

		default:
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
		importo = importo + 20;
		EXTI_clear(EXTI10);
	}
}

//Interrupt sul pin 4, pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		importo = importo + 5;
		EXTI_clear(EXTI4);
	}
}


void EXTI9_5_IRQHandler(void){
	//Interrupt sul pin 5, pulsante Z
	if(EXTI_isset(EXTI5)){
			importo = 0;
			EXTI_clear(EXTI5);
		}
	//Interrupt sul pin 6, pulsante T
	if(EXTI_isset(EXTI6)){
			if(current_state == INIT){
				quantita_carburante = importo * 150;
				DISPLAY_dp(1,1);
				current_state = EROGAZIONE;
			}

			if(current_state == STOP){
				current_state = IDLE;
			}
			EXTI_clear(EXTI6);
		}
}


void TIM2_IRQHandler(void){
	if(TIM_update_check(TIM2)){
		switch(current_state){
			case EROGAZIONE:
				count_time ++;
				if(count_time == 1){
					//Lampeggio del led giallo ogni 500 ms
					count_time = 0;
					GPIO_toggle(GPIOC,2);
				}
				break;

		}
		TIM_update_clear(TIM2);
	}
}

