#include "stm32_unict_lib.h"
#include <stdio.h>
char s[5];


int get_password(char* password){
  int i = 0;
  int same = 1;
  for(;;){
    char c = readchar();
    if(c == 8){
      if(i > 0){
        --i;
        __io_putchar(8);
        __io_putchar(' ');
        __io_putchar(8);
      }
    }else if(c == 13){
      printf("\n");
      return same;
    }else{
      if(c != password[i]){
        same = 0;
      }
      __io_putchar(c);
      i++;
    }
  }
}


typedef enum{
	ATTIVAZIONE,
	DISATTIVAZIONE,
	PRE_ALLARME,
	ALLARME,
	PRE_ATTIVAZIONE,
	SETUP
}state;

state current_state = DISATTIVAZIONE;
char *password;

void setup(void)
{
	password = "1234";
	//Per display e pulsanti
	GPIO_init(GPIOB);
	//Per i led
	GPIO_init(GPIOC);


	//X - Pulsante di attivazione allarme
	GPIO_config_input(GPIOB, 10);
	GPIO_config_EXTI(GPIOB, EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);

	//Y - Pulsante di disattivazione allarme
	GPIO_config_input(GPIOB, 4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);

	//Z - Pulsante che simula l'attivazione del sensore di intrusione
	GPIO_config_input(GPIOB, 5);
	GPIO_config_EXTI(GPIOB, EXTI5);
	EXTI_enable(EXTI5, FALLING_EDGE);

	//led ROSSO -> state = ATTIVAZIONE
	GPIO_config_output(GPIOB, 0);
	GPIO_write(GPIOB, 0, 0);

	//led GIALLO -> INTRUSO
	GPIO_config_output(GPIOC, 2);
	GPIO_write(GPIOC, 2, 0);

	//led VERDE -> DISATTIVATO
	GPIO_config_output(GPIOC, 3);
	GPIO_write(GPIOC, 3, 0);

	//Timer per segnalare le tempistiche
	TIM_init(TIM2);
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_config_timebase(TIM2, 8400, 2000); //interrupt ogni 200ms
	TIM_set(TIM2, 0);
	//TIM_off(TIM2);
	TIM_on(TIM2);

	//UART
	CONSOLE_init();
}

int count_pre_allarme = 0 ;
int count_tempo_uscita = 0;
int count_allarme = 0;

void loop(void)
{
	switch(current_state){
		case ATTIVAZIONE:
			//spengo il led verde
			GPIO_write(GPIOC, 3, 0);
			printf("Sono in attivazione...\n");
			//accendo led rosso
			GPIO_write(GPIOB, 0, 1);

			break;
		case DISATTIVAZIONE:
			//setup();
			//CAMBIO DELLA PASSWORD
			printf("Disattivazione\n");
			GPIO_write(GPIOB, 0, 0);
			GPIO_write(GPIOC, 2, 0);
			GPIO_write(GPIOC, 3, 1);

			/*if(get_password("!")){
				current_state = SETUP;
			}*/
			if (kbhit()) {
				char c = readchar();
				if(c == '!'){
					current_state = SETUP;
				}
			}
			break;
		case PRE_ALLARME:
		{
			GPIO_write(GPIOC, 3, 0);
			int count = 0;
			//L'utente avrà 20 secondi per poter inserire la password
			for(int i = 0; i<3; i++){
				printf("[PRE_ALLARME] Inserisci la password \n");
				if(get_password(password)){
						current_state = DISATTIVAZIONE;
						break;
				}
				count ++;
			}

			if(count >= 3 ){
				current_state = ALLARME;
			}


			break;
		}
		case ALLARME:
			//deve lampeggiare il led rosso
			GPIO_write(GPIOC, 2, 0);
			printf("Inserisci la password \n");
			if(get_password(password)){
				current_state = DISATTIVAZIONE;
			}

			break;
		case SETUP:
			printf("Scrivi il comando da eseguire :");

			break;
	}

}

//Premo pulsante X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		//richiedere nella UART la password di 4 caratteri numerici
		printf("Inserisci la password\n");
		if(get_password(password)){
			current_state = PRE_ATTIVAZIONE;
		}
		EXTI_clear(EXTI10);
	}
}

//Premo pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		int count = 0;
		//richiedi password e passsa allo stato di disattivazione

		for(int i = 0; i<3; i++){
			printf("Inserisci la password per disattivare \n");
			if(get_password(password)){
				current_state = DISATTIVAZIONE;
				break;
			}
			count ++;
		}


		if(count >= 3 ){
			//aiuto qualcuno sta sbagliando!!!
			current_state = ALLARME;
		}
		EXTI_clear(EXTI4);
	}
}

void EXTI9_5_IRQHandler(void){
	//Premo pulsante Z
	if(EXTI_isset(EXTI5)){
		if(current_state == ATTIVAZIONE){
			//presenza di un intruso
			GPIO_write(GPIOC, 2, 1);
			GPIO_write(GPIOB, 0, 0);
			current_state = PRE_ALLARME;
		}
		EXTI_clear(EXTI5);
	}
}
void TIM2_IRQHandler(void){
	if(TIM_update_check(TIM2)){
		switch(current_state){
			case PRE_ATTIVAZIONE:
				printf("[TIMER] Pre-attivazione \n");
				GPIO_toggle(GPIOC, 3);
				count_tempo_uscita++;
				if(count_tempo_uscita %25 == 0){
					current_state = ATTIVAZIONE;
				}


				break;
			case PRE_ALLARME:
				printf("[TIMER] Pre-allarme \n");
				count_pre_allarme ++;
				if(count_pre_allarme %100 == 0){
					current_state = ALLARME;
				}
				break;
			case ALLARME:

				GPIO_toggle(GPIOB, 0);
				break;

		}
		TIM_update_clear(TIM2);
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

