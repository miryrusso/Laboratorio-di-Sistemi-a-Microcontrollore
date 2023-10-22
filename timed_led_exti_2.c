#include "stm32_unict_lib.h"
#include <stdio.h>

typedef enum {
	st_OFF,
	st_ON,
	st_CONFIG
} t_state;

t_state current_state;

int timer;
int max_timer;

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

	GPIO_config_input(GPIOB,4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);

	GPIO_config_input(GPIOB,5);
	GPIO_config_EXTI(GPIOB, EXTI5);
	EXTI_enable(EXTI5, FALLING_EDGE);

	GPIO_config_input(GPIOB,6);
	GPIO_config_EXTI(GPIOB, EXTI6);
	EXTI_enable(EXTI6, FALLING_EDGE);


	max_timer = 1000;
	current_state = st_OFF;
}

void loop(void)
{
	char s[5];

	switch(current_state) {

	case st_OFF:
		GPIO_write(GPIOB, 0, 0); // led spento
		GPIO_write(GPIOC, 2, 0);
		break;

	case st_ON:
		GPIO_write(GPIOB, 0, 1); // led acceso

		sprintf(s, "%4d", timer / 100);
		DISPLAY_puts(0,s);

		delay_ms(10);

		++timer;
		if (timer == max_timer)
			current_state = st_OFF;

		break;

	case st_CONFIG:
		GPIO_write(GPIOC, 2, 1);
		sprintf(s, "%4d", max_timer / 100);
		DISPLAY_puts(0,s);
		break;
	}

}

void EXTI15_10_IRQHandler(void)
{
	// PB X
	if (EXTI_isset(EXTI10)) {
		timer = 0;
		current_state = st_ON;
		EXTI_clear(EXTI10);
	}
}

void EXTI9_5_IRQHandler(void)
{
	// PB Z
	if (EXTI_isset(EXTI5)) {
		if (current_state == st_CONFIG) {
			max_timer -= 100;
		}
		EXTI_clear(EXTI5);
	}
	// PB T
	if (EXTI_isset(EXTI6)) {
		if (current_state == st_CONFIG) {
			max_timer += 100;
		}
		EXTI_clear(EXTI6);
	}
}

void EXTI4_IRQHandler(void)
{
	// PB Y
	if (EXTI_isset(EXTI4)) {
		switch(current_state) {
		case st_OFF:
			current_state = st_CONFIG;
			break;
		case st_CONFIG:
			current_state = st_OFF;
			break;
		}
		EXTI_clear(EXTI4);
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
