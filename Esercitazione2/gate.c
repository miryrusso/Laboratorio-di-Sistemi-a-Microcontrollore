#include "stm32_unict_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
  APRENDO,
  APERTO,
  CHIUDENDO,
  CHIUSO
}state;

state current_state;
char s[5];
int count_irq;


void setup(void)
{
  //Per display e pulsanti
  GPIO_init(GPIOB);
  //Per i led
  DISPLAY_init();

  //X
  GPIO_config_input(GPIOB, 10);
  GPIO_config_EXTI(GPIOB, EXTI10);
  EXTI_enable(EXTI10, FALLING_EDGE);
  //Y
  GPIO_config_input(GPIOB, 4);
  GPIO_config_EXTI(GPIOB, EXTI4);
  EXTI_enable(EXTI4, FALLING_EDGE);
  //Z
  GPIO_config_input(GPIOB, 5);
  GPIO_config_EXTI(GPIOB, EXTI5);
  EXTI_enable(EXTI5, FALLING_EDGE);

  //led ROSSO
  GPIO_config_output(GPIOB, 0);
  GPIO_write(GPIOB, 0, 0);

  sprintf(s, "----");
  DISPLAY_puts(0, s);

  //imposto il timer: divido la sequenza per 8400, così avrò una frequenza di 10000Hz/s,
  ///con un interrupt ogni 5000hz (mezzo secondo);
  TIM_init(TIM2);
  TIM_config_timebase(TIM2, 8400, 5000);
  TIM_enable_irq(TIM2, IRQ_UPDATE);
  TIM_off(TIM2);

  //conto gli interrupt per "segnare il tempo"
  count_irq = 0;

  current_state = CHIUSO;


}

void loop(void)
{
  switch(current_state){
    case APRENDO:
      if(count_irq >=  0 && count_irq < 5){
          sprintf(s,"----");
        }
      if(count_irq >=  5 && count_irq < 10){
        sprintf(s,"--- ");
      }
      if(count_irq >=  10 && count_irq < 15){
        sprintf(s, "--  ");
      }
      if(count_irq >=  15 && count_irq < 20){
        sprintf(s, "-   ");
      }
      if(count_irq == 20){
        sprintf(s, "    ");
        count_irq = 0;
        current_state = APERTO;
      }
      break;
    case CHIUDENDO:
      if(count_irq == 0){
        sprintf(s, "----");
        setup();
      }
      if(count_irq >  0 && count_irq <= 5){
        sprintf(s,"----");
      }
      if(count_irq >  5 && count_irq <= 10){
        sprintf(s,"--- ");
      }
      if(count_irq >  10 && count_irq <= 15){
        sprintf(s, "--  ");
      }
      if(count_irq >  15 && count_irq <= 20){
        sprintf(s, "-   ");
      }
      break;
    case APERTO:
      if(count_irq == 8){
        count_irq = 20;
        current_state = CHIUDENDO;
      }
      break;
  }
  DISPLAY_puts(0,s);
}

//Premo pulsante X
void EXTI15_10_IRQHandler(void){
  if(EXTI_isset(EXTI10)){
    if(current_state == CHIUSO){
      current_state = APRENDO;
      TIM_on(TIM2);
    }
    if(current_state == CHIUDENDO){
      current_state = APRENDO;
    }
    EXTI_clear(EXTI10);
  }
}

//Premo pulsante Y
void EXTI4_IRQHandler(void){
  if(EXTI_isset(EXTI4)){
    if(current_state == APERTO){
      count_irq = 20;
      current_state = CHIUDENDO;
    }
    EXTI_clear(EXTI4);
  }
}

void EXTI9_5_IRQHandler(void){
  //Premo pulsante Z
  if(EXTI_isset(EXTI5)){
    if(current_state == APERTO){
      count_irq = 0;
    }
    if(current_state == CHIUDENDO){
      current_state = APRENDO;
    }
    EXTI_clear(EXTI5);
  }
}

void TIM2_IRQHandler(void){
  if(TIM_update_check(TIM2)){
    if(current_state == APRENDO || current_state == APERTO) count_irq++;
    if(current_state == CHIUDENDO) count_irq--;
    GPIO_toggle(GPIOB, 0);
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

