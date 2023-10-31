#include "stm32_unict_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
	APRENDO,
	CHIUDENDO,
	MOVIMENTO,
	ATTESA,
	FERMO
}state;


int piano = 0;
int destinazione = 0;
state current_state = FERMO;
int count_floors = 0;
int count_start = 0;
int count_open = 0;
int count_close = 0;
char s[5];


void setup(void)
{
	//Per display e pulsanti
	GPIO_init(GPIOB);
	//Per i led
	GPIO_init(GPIOC);


	//X - Piano 0
	GPIO_config_input(GPIOB, 10);
	GPIO_config_EXTI(GPIOB, EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);
	//Y - Piano 1
	GPIO_config_input(GPIOB, 4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);
	//Z - Piano 2
	GPIO_config_input(GPIOB, 5);
	GPIO_config_EXTI(GPIOB, EXTI5);
	EXTI_enable(EXTI5, FALLING_EDGE);
	//led ROSSO
	GPIO_config_output(GPIOB, 0);
	GPIO_write(GPIOB, 0, 0);
	//led GIALLO
	GPIO_config_output(GPIOC, 2);
	GPIO_write(GPIOC, 2, 0);
	//led VERDE
	GPIO_config_output(GPIOC, 3);
	GPIO_write(GPIOC, 3, 0);

	DISPLAY_init();
	sprintf(s, "%4d", piano);
	DISPLAY_puts(0, s);

	TIM_init(TIM2);
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_config_timebase(TIM2, 8400, 2500); //interrupt ogni 250ms
	TIM_set(TIM2, 0);
	TIM_off(TIM2);

	//ADC
	 ADC_init(ADC1, ADC_RES_6, ADC_ALIGN_RIGHT);
	    ADC_channel_config(ADC1, GPIOC, 0, 10);
	    ADC_on(ADC1);
	    ADC_sample_channel(ADC1, 10);


	  ADC_init(ADC1, ADC_RES_6, ADC_ALIGN_RIGHT);
	       ADC_channel_config(ADC1, GPIOC, 0, 11);
	       ADC_on(ADC1);
	       ADC_sample_channel(ADC1, 11);

}



void loop(void)
{
	DISPLAY_puts(0, s);

}

//Premo pulsante X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		if(current_state == FERMO){
			destinazione = 2;
			if(destinazione != piano){
				current_state = ATTESA;
				TIM_on(TIM2);
			}
		}
		EXTI_clear(EXTI10);
	}
}

//Premo pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		if(current_state == FERMO){
			destinazione = 1;
			if(destinazione != piano){
				current_state = ATTESA;
				TIM_on(TIM2);
			}
		}
		EXTI_clear(EXTI4);
	}
}

void EXTI9_5_IRQHandler(void){
	//Premo pulsante Z
	if(EXTI_isset(EXTI5)){
		if(current_state == FERMO){
			destinazione = 0;
			if(destinazione != piano){
				current_state = ATTESA;
				TIM_on(TIM2);
			}
		}
		EXTI_clear(EXTI5);
	}
}

void TIM2_IRQHandler(void){
	if(TIM_update_check(TIM2)){
		switch(current_state){
			case MOVIMENTO:
				count_floors++;
				GPIO_toggle(GPIOB, 0);
				move();
				break;
			case ATTESA:
				count_start++;
				if(count_start % 2 == 0){
					current_state = CHIUDENDO;
				}
				break;
			case CHIUDENDO:
				GPIO_toggle(GPIOC, 2);
				count_close++;
				if(count_close % 6 == 0){
					GPIO_write(GPIOC, 2, 0);
					current_state = MOVIMENTO;
				}
				sprintf(s, "%4d", piano);
				break;
			case APRENDO:
				GPIO_toggle(GPIOC, 3);
				count_open++;
				if(count_open % 6 == 0){
					GPIO_write(GPIOC, 3, 0);
					current_state = FERMO;
				}
				sprintf(s, "%4d", piano);
				break;
			case FERMO:
				GPIO_write(GPIOB, 0, 0);
				GPIO_write(GPIOC, 2, 0);
				GPIO_write(GPIOC, 3, 0);
				TIM_off(TIM2);
				break;
		}
		TIM_update_clear(TIM2);
	}
}

void move(){
	if(count_floors % 16 == 0  && destinazione > piano) {
		piano++;
		sprintf(s, "%4d", piano);
	}
	else if(count_floors % 16 == 0 && destinazione < piano) {
		piano--;
		sprintf(s, "%4d", piano);
	}
	else if(destinazione == piano){
		current_state = APRENDO;
		GPIO_write(GPIOB, 0, 0);
	}
	else{
		if(destinazione > piano) sprintf(s, "  %d-", piano);
		else sprintf(s, "%4d-", piano-1);
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

