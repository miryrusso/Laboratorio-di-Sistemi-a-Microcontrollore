#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stm32_unict_lib.h"

enum {
    READY, //Mostra lo stato "Ready" sul display, accende il LED verde e imposta un tempo casuale per l'esecuzione successiva
	WAIT, //Mostra lo stato "E1" sul display e accende il LED rosso.
	SHOW, //Mostra il tempo trascorso dallo stato "E1" sul display.
    END //Mostra il tempo trascorso dopo lo stato "E2" sul display e accende il LED giallo.
};

int state = READY;
char s[5];
int t_time = 0;
int rand_time = 0;

void setup()
{
    GPIO_init(GPIOB);
    GPIO_init(GPIOC);
    TIM_init(TIM2);
    DISPLAY_init();

    //pushbutton X
    GPIO_config_input(GPIOB, 10);
    GPIO_config_EXTI(GPIOB, EXTI10);
    EXTI_enable(EXTI10, FALLING_EDGE);

    //led giallo
    GPIO_config_output(GPIOC, 2);

    //led rosso
    GPIO_config_output(GPIOB, 0);

    //led verde
    GPIO_config_output(GPIOC, 3);

    //timer
    TIM_config_timebase(TIM2, 4200, 20);
    //Frequenza effettiva = 84,000,000 Hz / (4200 * 20) = 1,000 Hzt
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);
}

void loop()
{
    switch(state)
    {
        case READY:
        	if(GPIO_read(GPIOC,2)){
        		GPIO_write(GPIOC,2,0);
        	}
	        sprintf(s, "%4s", "RUN");
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOC,3,1);
			rand_time = rand()%6000+2000;
	        t_time = 0;
            break;

        case WAIT:
        	sprintf(s, "%4s", "ATT");
	        DISPLAY_puts(0,s);
			GPIO_write(GPIOC,3,0);
			GPIO_write(GPIOB,0,1);
	        break;

        case SHOW:
        	sprintf(s, "%4d", t_time);
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOB,0,0);
			break;

        case END:
        	//visualizzo sullo schermetto il timer
        	sprintf(s, "%4d", t_time);
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOC,2,1);
        	break;

    }
}

//pushbutton X
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_isset(EXTI10)) {
		switch (state) {
		case READY:
			state = WAIT;
			break;
		case SHOW:
			state = END;
			break;
        case END:
            state = READY;
            break;
		}
		EXTI_clear(EXTI10);
	}
}

//timer
void TIM2_IRQHandler(void)
{
	if(TIM_update_check(TIM2))
	{
		if(state == WAIT)
		{
			t_time++;
			if (t_time > rand_time){
				state = SHOW;
				t_time = 0;
			}
		}
		else if (state == SHOW)
		{
			t_time++;
		}

		TIM_update_clear(TIM2);
	}
}

int main()
{
	srand(time(NULL));
    setup();
    for(;;)
    {
        loop();
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stm32_unict_lib.h"

enum {
    READY, //Mostra lo stato "Ready" sul display, accende il LED verde e imposta un tempo casuale per l'esecuzione successiva
	WAIT, //Mostra lo stato "E1" sul display e accende il LED rosso.
	SHOW, //Mostra il tempo trascorso dallo stato "E1" sul display.
    END //Mostra il tempo trascorso dopo lo stato "E2" sul display e accende il LED giallo.
};

int state = READY;
char s[5];
int t_time = 0;
int rand_time = 0;

void setup()
{
    GPIO_init(GPIOB);
    GPIO_init(GPIOC);
    TIM_init(TIM2);
    DISPLAY_init();

    //pushbutton X
    GPIO_config_input(GPIOB, 10);
    GPIO_config_EXTI(GPIOB, EXTI10);
    EXTI_enable(EXTI10, FALLING_EDGE);

    //led giallo
    GPIO_config_output(GPIOC, 2);

    //led rosso
    GPIO_config_output(GPIOB, 0);

    //led verde
    GPIO_config_output(GPIOC, 3);

    //timer
    TIM_config_timebase(TIM2, 4200, 20);
    //Frequenza effettiva = 84,000,000 Hz / (4200 * 20) = 1,000 Hzt
    TIM_enable_irq(TIM2, IRQ_UPDATE);
    TIM_on(TIM2);
}

void loop()
{
    switch(state)
    {
        case READY:
        	if(GPIO_read(GPIOC,2)){
        		GPIO_write(GPIOC,2,0);
        	}
	        sprintf(s, "%4s", "RUN");
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOC,3,1);
			rand_time = rand()%6000+2000;
	        t_time = 0;
            break;

        case WAIT:
        	sprintf(s, "%4s", "ATT");
	        DISPLAY_puts(0,s);
			GPIO_write(GPIOC,3,0);
			GPIO_write(GPIOB,0,1);
	        break;

        case SHOW:
        	sprintf(s, "%4d", t_time);
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOB,0,0);
			break;

        case END:
        	//visualizzo sullo schermetto il timer
        	sprintf(s, "%4d", t_time);
	        DISPLAY_puts(0,s);
	        GPIO_write(GPIOC,2,1);
        	break;

    }
}

//pushbutton X
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_isset(EXTI10)) {
		switch (state) {
		case READY:
			state = WAIT;
			break;
		case SHOW:
			state = END;
			break;
        case END:
            state = READY;
            break;
		}
		EXTI_clear(EXTI10);
	}
}

//timer
void TIM2_IRQHandler(void)
{
	if(TIM_update_check(TIM2))
	{
		if(state == WAIT)
		{
			t_time++;
			if (t_time > rand_time){
				state = SHOW;
				t_time = 0;
			}
		}
		else if (state == SHOW)
		{
			t_time++;
		}

		TIM_update_clear(TIM2);
	}
}

int main()
{
	srand(time(NULL));
    setup();
    for(;;)
    {
        loop();
    }
}

