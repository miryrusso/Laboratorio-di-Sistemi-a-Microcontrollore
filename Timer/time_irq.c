#include "stm32_unict_lib.h"
#include <stdio.h>

void setup(void)
{
	//inizializzazione del Display
	DISPLAY_init();

	//inizializzazione GPIO di tipo B
	GPIO_init(GPIOB);

	//Configurazione in output del led 0
	GPIO_config_output(GPIOB, 0);


	TIM_init(TIM2);
	//Configurazione del timer dove:
	//								1) Tipologia di Timer
	//								2) Prescaler, divisione
	//								3) Max Value
	TIM_config_timebase(TIM2, 8400, 5000);

	TIM_enable_irq(TIM2, IRQ_UPDATE);
	//accendiamo il timer
	TIM_on(TIM2);
}

void loop(void)
{
	char s[5];
	//Stampiamo l'incremento basandoci sul clock di input (CNT)
	sprintf(s, "%4d", TIM2->CNT);
	DISPLAY_puts(0,s);
}

void TIM2_IRQHandler(void)
{
	//Verifica se accade l'evento e quindi si è arrivati al max value
	if (TIM_update_check(TIM2)) {
		//lampeggio led rosso
		GPIO_toggle(GPIOB,0);
		//end event
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

