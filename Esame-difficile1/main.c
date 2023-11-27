/**
 * Si consideri un sistema di controllo di un braccio meccanico operante su un piano XY .
Il sistema deve permettere la movimentazione degli assi verso un punto target impostato
manualmente; si supponga che la dimensione del piano vada da 0 cm a 99 cm per entrambi gli
assi.
Il sistema deve funzionare nel seguente modo:

• Condizione normale: i valori della posizione attuale degli assi vanno visalizzati sul display
(prime due cifre per l’asse X e seconde due cifre per l’asse Y);

• Tasto “X”: il sistema entra in modalità “target” permettendo, tramite AN11 (asse X) e
AN10 (asse Y) di impostare il punto target; in questa condizione, i valori dei due target
vanno visalizzati sul display (prime due cifre per l’asse X e seconde due cifre per l’asse
Y); premendo il tasto “X” nuovamente si avvia il moto, premendo il tasto “Y” si annulla
l’inserimento e si ritorna in condizione normale;

• Il moto verso il target deve avvenire ad una velocità v espressa in cm/s (vedi oltre); il
modo deve arrestarsi quando gli assi sono entro 0.2 cm dalla posizione target specificata;
il moto deve essere gestito con un timer con granularità di 10 ms;

• Durante il moto, il LED rosso lampeggia con periodo di 200 ms, il LED giallo si accende
quando si è entro 5 cm dal target, mentre il LED verde si accende quando si è raggiunto il
target;

• Il tasto “Z” permette l’azzeramento degli assi, in questa situazione viene impostato come
target di default il punto (0, 0) e il moto viene avviato.

• Il tasto “T” permette l’ingresso del sistema nella modalità di configurazione in cui, attraverso
la UART, è possibile inviare uno dei seguenti comandi:

• C valore, imposta il nuovo valore velocità v nel range di interi [1, 5] cm/s;
• EXIT, esce dallo stato di configurazione.
Come valore iniziale della velocità usare v = 2 cm/s.

*/

#include "stm32_unict_lib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_STRING 40
#define COORDINATA_MINIMA 0
#define COORDINATA_MASSIMA 99

char s[5];
int v = 2;
double x = 0;
double y = 0;
double space = 0;
int timer = 0;
int count = 0;
int dest_X = 0;
int dest_Y = 0;



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
	IDLE,
	NORMALE,
	TARGET,
	MOVIMENTO,
	SETUP,
}state;

state current_state = NORMALE;
state last_state = NORMALE;



void move(int dest_X, int dest_Y){
	double theta = atan2(dest_Y-y, dest_X-x);
	//double theta = atan2(dest_Y-y, dest_X-x);
	//int theta = 45;
	printf("%d\n", theta);
	space = (double)v * 0.001;
	//space = 0.02;
//	printf("space = %d", 1);
	double incremento_cos = space * cos(theta);
	double incremento_sin = space * sin(theta);

	printf("Incremento cos = %d Incremento_sin = %d \n ", incremento_cos, incremento_sin);


	while((double)(sqrt(pow(dest_Y - y, 2) + pow(dest_X - x, 2))) > 0.2) {
		printf("Sono dentro il while...\n");
		if(timer >= 1){
			timer = 0;
			x+= incremento_cos;
			y+= incremento_sin;
		}
		printf( " X = %d Y= %d", (int)x,(int)y);
		/**il LED giallo si accende
quando si è entro 5 cm dal target,*/
		if(sqrt(pow(dest_Y - y, 2) + pow(dest_X - x, 2)) < 5){
			GPIO_write(GPIOC, 2, 1);
		}
	}

	timer = 0;
	current_state = NORMALE;

}

void setup(void)
{
	//password = "1234";
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

	//T
	GPIO_config_input(GPIOB, 6);
	GPIO_config_EXTI(GPIOB, EXTI6);
	EXTI_enable(EXTI6, FALLING_EDGE);

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
	TIM_config_timebase(TIM2, 8400, 100); //interrupt ogni 10ms
	TIM_set(TIM2, 0);
	//TIM_off(TIM2);
	TIM_on(TIM2);

	//UART
	CONSOLE_init();

	//trimmer ADC_11 per asse X
	ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
	ADC_channel_config(ADC1, GPIOC, 1, 11);
	ADC_on(ADC1);

	//trimmer ADC_10 per asse Y
	ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
	ADC_channel_config(ADC1, GPIOC, 0, 10);
	ADC_on(ADC1);
}

void loop(void)
{
	if (kbhit()) {
		char c = readchar();
		if(c == 'T'){
			current_state = SETUP;
		}
	}

	switch(current_state){
		case IDLE:
		{




			break;

		}
		case NORMALE:
			//i valori della posizione attuale degli assi vanno visalizzati sul display
			//(prime due cifre per l’asse X e seconde due cifre per l’asse Y);
			//printf("[NORMALE]..\n");
			last_state = NORMALE;
			sprintf(s, "%2d%2d", (int)x,(int)y);
			DISPLAY_puts(0,s);
			if(last_state == MOVIMENTO){
				GPIO_write(GPIOB, 0,1);
				GPIO_write(GPIOB, 2,0);
				GPIO_write(GPIOC, 3,1);
			}
			break;

		case TARGET:
		{

			/*il sistema entra in modalità “target” permettendo, tramite AN11 (asse X) e
			AN10 (asse Y) di impostare il punto target; in questa condizione, i valori dei due target
			vanno visalizzati sul display (prime due cifre per l’asse X e seconde due cifre per l’asse
			Y); premendo il tasto “X” nuovamente si avvia il moto, premendo il tasto “Y” si annulla
			l’inserimento e si ritorna in condizione normale;*/
			printf("[TARGET]\n");
			last_state = TARGET;
			//sample from AN11 asse X
			ADC_sample_channel(ADC1, 11);
			ADC_start(ADC1);
			while(!ADC_completed(ADC1)) {}
			dest_X = ADC_read(ADC1);
			//ARR = ADCVAL * ((VALORE_MASSIMO - VALORE MINIMO)/255) + VALORE_MINIMO
			dest_X = (dest_X/255.0)*99;

			//sample from AN10 asse Y
			ADC_sample_channel(ADC1, 10);
			ADC_start(ADC1);
			while(!ADC_completed(ADC1)) {}
			dest_Y = ADC_read(ADC1);
			//ARR = ADCVAL * ((VALORE_MASSIMO - VALORE MINIMO)/255) + VALORE_MINIMO
			dest_Y = (dest_Y/255.0)*99;

			sprintf(s, "%2d%2d", dest_X,dest_Y);
			DISPLAY_puts(0,s);

			break;
		}

		case MOVIMENTO:
			printf("[MOVIMENTO]\n");
			/**
			 * • Il moto verso il target deve avvenire ad una velocità v espressa in cm/s (vedi oltre);
			 * il moTo deve arrestarsi quando gli assi sono entro 0.2 cm dalla posizione target
			 * specificata; */
			last_state = MOVIMENTO;
			move(dest_X,dest_Y);
			break;
		case SETUP:
			printf("[SETUP]\n");
			/*• C valore, imposta il nuovo valore velocità v nel range di interi [1, 5] cm/s;
				• EXIT, esce dallo stato di configurazione.*/

			GPIO_write(GPIOC, 3, 0);

			printf("Scrivi il comando da eseguire [C-VALORE, EXIT] :\n");
			getstring(s, MAX_STRING);

			if(!strcmp(s, "C-VALORE")){
				do{
					printf("Inserisci il nuovo valore della velocità [1,5]..\n");
					getstring(s, MAX_STRING);
					v = atoi(s);
				}while(v<1 || v>5);

				//current_state = NORMALE;
			}

			if(!strcmp(s, "EXIT")){
				current_state = NORMALE;
			}

			break;
	}

}

//Premo pulsante X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		if(last_state == NORMALE)
			current_state = TARGET; //entra in modalità TARGET
		if(last_state == TARGET)
					current_state = MOVIMENTO; //entra in modalità TARGET
		EXTI_clear(EXTI10);
	}
}

//Premo pulsante Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		if(current_state == TARGET){
			current_state = NORMALE;
			last_state = NORMALE;
		}

		EXTI_clear(EXTI4);
	}
}




void EXTI9_5_IRQHandler(void){
	//Premo pulsante Z PA10
	if(EXTI_isset(EXTI5)){
		dest_X = 0;
		dest_Y = 0;
		current_state = MOVIMENTO;
		EXTI_clear(EXTI5);
	}

	if(EXTI_isset(EXTI6)){
			current_state = SETUP;
			EXTI_clear(EXTI6);
	}
}



void TIM2_IRQHandler(void){
	if(TIM_update_check(TIM2)){
		switch(current_state){
			case MOVIMENTO:
				timer++;
				count++;
				if(count >= 20){
					count = 0;
					GPIO_toggle(GPIOC, 3);
				}

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

