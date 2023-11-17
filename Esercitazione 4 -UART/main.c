#include "stm32_unict_lib.h"
#include <stdio.h>
#include <string.h>

#define MAX_STRING 40

void getstring(char * s, int maxlen)
{
	int i = 0;
	for (;;) {
		char c = readchar();
		if (c == 13) {
			printf("\n");
			s[i] = 0;
			return;
		}
		else if (c == 8) {
			if (i > 0) {
				--i;
				__io_putchar(8); // BS
				__io_putchar(' '); // SPAZIO
				__io_putchar(8); // BS
			}
		}
		else if (c >= 32) { // il carattere appartiene al set stampabile
			if (i < maxlen) {
				__io_putchar(c); // echo del carattere appena inserito
				// inserisci il carattere nella stringa
				s[i] = c;
				i++;
			}
		}
	}
}


typedef enum{
	ATTIVAZIONE,
	DISATTIVAZIONE,
	PRE_ALLARME,
	ALLARME,
	PRE_ATTIVAZIONE,
	SETUP,
	RICHIESTA
}state;

state current_state = DISATTIVAZIONE;
state last_state = DISATTIVAZIONE;
char *password = NULL;
char schermetto[5];
char s[5];
int out_time = 25;
int in_time = 100;

void setup(void)
{
	//password = "1234";
	password = (char *)malloc(sizeof(char) * (4 + 1)); // +1 per il terminatore '\0'
	strcpy(password, "1234");
	//Per display e pulsanti
	GPIO_init(GPIOB);
	//Per i led
	GPIO_init(GPIOC);

	//Display
	DISPLAY_init();


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
int count = 0;

void loop(void)
{
	switch(current_state){
		case RICHIESTA:
		{



			if(last_state == ATTIVAZIONE){
				printf("Inserisci la password per disattivare\n");
			}else{
				printf("Inserisci la password\n");
			}

			getstring(s, MAX_STRING);
			if(!strcmp(s, password)){
				if(last_state == DISATTIVAZIONE){
					current_state = PRE_ATTIVAZIONE;

				}else if(last_state == ATTIVAZIONE){
					current_state = DISATTIVAZIONE;
				}

			}else{
				count ++;
				if(count > 2){
					current_state = ALLARME;
				}
			}
			last_state = RICHIESTA;

			break;

		}
		case ATTIVAZIONE:
			last_state = ATTIVAZIONE;
			//spengo il led verde
			sprintf(schermetto, "%4s", "ATT");
			DISPLAY_puts(0,schermetto);
			GPIO_write(GPIOC, 3, 0);
			//printf("Sono in attivazione...\n");
			//accendo led rosso
			GPIO_write(GPIOB, 0, 1);

			break;
		case DISATTIVAZIONE:
			last_state = DISATTIVAZIONE;
			//setup();
			//CAMBIO DELLA PASSWORD
			sprintf(schermetto, "%4s", "DIS");
			DISPLAY_puts(0,schermetto);
			//printf("Disattivazione\n");
			GPIO_write(GPIOB, 0, 0);
			GPIO_write(GPIOC, 2, 0);
			GPIO_write(GPIOC, 3, 1);
			if (kbhit()) {
				char c = readchar();
				if(c == '!'){
					current_state = SETUP;
				}
			}
			break;
		case PRE_ALLARME:
		{
			GPIO_write(GPIOB, 0, 0);
			last_state = PRE_ALLARME;
			sprintf(schermetto, "%4s", "PRE");
			DISPLAY_puts(0,schermetto);

			int count = 0;
				if(kbhit()){ //altrimenti getstring resta in loop
					getstring(s, MAX_STRING);
									if(!strcmp(password, s)){
											current_state = DISATTIVAZIONE;
									}else{
										count ++;
										if(count > 3 ){
											current_state = ALLARME;
										}
									}
				}

			break;
		}
		case ALLARME:
			last_state = ALLARME;
			char* pipo = "ALL";
			sprintf(schermetto, "%4s", pipo);
			DISPLAY_puts(0,schermetto);
			//deve lampeggiare il led rosso
			//GPIO_write(GPIOC, 2, 0);
			printf("Inserisci la password \n");
			getstring(s, MAX_STRING);
			if(!strcmp(password, s)){
				current_state = DISATTIVAZIONE;
			}

			break;
		case SETUP:
			last_state = SETUP;
			GPIO_write(GPIOC, 3, 0);
			printf("Scrivi il comando da eseguire :\n");
			getstring(s, MAX_STRING);

			if(!strcmp(s, "PASSWORD")){
				printf("Inserisci la nuova password..\n");
				for(int i = 0; i<4; i++){
					char c = readchar();
					s[i] = c;
				}
				printf("[PASSWORD VECCHIA] %s\n", password);
				strncpy(password, s, strlen(password));
				printf("[PASSWORD NUOVA] %s \n", password);
				//current_state = DISATTIVAZIONE;

			}else if(!strcmp(s, "OUT-TIME")){

				printf("Inserisci valore OUT-TIME..\n");
				getstring(s, MAX_STRING);
				out_time = atoi(s);
				printf("Nuovo out-time %d\n", out_time);

			}else if(!strcmp(s, "IN-TIME")){
				printf("Inserisci valore IN-TIME..\n");
				getstring(s, MAX_STRING);
				in_time = atoi(s);
				printf("Nuovo in-time %d\n", in_time);
			}

			if(!strcmp(s, "EXIT")){
				current_state = DISATTIVAZIONE;
			}

			break;
	}

}

//Premo pulsante X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
			current_state = RICHIESTA;
		EXTI_clear(EXTI10);
	}
}

//Premo pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		if(current_state == ATTIVAZIONE){
			current_state = RICHIESTA;
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
				GPIO_toggle(GPIOC, 3);
				count_tempo_uscita++;
				if(count_tempo_uscita %out_time == 0){
					current_state = ATTIVAZIONE;
				}


				break;
			case PRE_ALLARME:
				count_pre_allarme ++;
				if(count_pre_allarme >= in_time){
					count_pre_allarme = 0;
					current_state = ALLARME;
				}
				break;
			case ALLARME:
				GPIO_write(GPIOC, 2, 0);
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

