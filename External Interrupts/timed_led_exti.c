#include "stm32_unict_lib.h"
#include <stdio.h>

typedef enum {
	st_OFF,
	st_ON
} t_state;

t_state current_state;

int pb_X_event;
int timer;

void setup(void)
{
	DISPLAY_init();

	GPIO_init(GPIOC);
	GPIO_config_output(GPIOC, 2);

	GPIO_init(GPIOB);
	GPIO_config_output(GPIOB, 0);

	GPIO_config_input(GPIOB,10);
	GPIO_config_EXTI(GPIOB, EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);

	current_state = st_OFF;
}

void loop(void)
{
	char s[5];

	switch(current_state) {

	case st_OFF:
		GPIO_write(GPIOB, 0, 0); // led spento
		if (pb_X_event) {
			pb_X_event = 0;
			timer = 0;
			current_state = st_ON;
		}
		break;

	case st_ON:
		GPIO_write(GPIOB, 0, 1); // led acceso

		sprintf(s, "%4d", timer / 100);
		DISPLAY_puts(0,s);

		delay_ms(10);

		++timer;
		if (timer == 1000)
			current_state = st_OFF;

		if (pb_X_event) {
			pb_X_event = 0;
			timer = 0;
		}
		break;
	}

}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_isset(EXTI10)) {
		pb_X_event = 1;
		EXTI_clear(EXTI10);
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
