//TODO : lo stesso tasto non può essere premuto più di 1 volta 
#include "stm32_unict_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define n 4

int ind = 0;
int value = 1;
int output[n];
int input;
char tasti[n][n] = {{'1', '4', '7', '*'}, {'2', '5', '8', '0'}, {'3', '6', '9', '#'}, {'A', 'B', 'C', 'D'}};
uint8_t bool = 0;
uint8_t bool_to_print = 0;
char last_c = 'X';
char current_c = 'X';

void setup(){
  CONSOLE_init();

  GPIO_init(GPIOA);
  GPIO_init(GPIOB);
  GPIO_init(GPIOC);

  GPIO_config_input(GPIOA, 8);
  GPIO_config_input(GPIOA, 9);
  GPIO_config_input(GPIOB, 10);
  GPIO_config_input(GPIOC, 7);
  GPIO_config_output(GPIOA, 10);
  GPIO_config_output(GPIOB, 3);
  GPIO_config_output(GPIOB, 4);
  GPIO_config_output(GPIOB, 5);

  //pull up per vedere quando cambia da 1 a 0 l'input
  GPIO_PULL_UP(GPIOA, 8);
  GPIO_PULL_UP(GPIOA, 9);
  GPIO_PULL_UP(GPIOB, 10);
  GPIO_PULL_UP(GPIOC, 7);

  TIM_init(TIM2);
  TIM_enable_irq(TIM2, IRQ_UPDATE);
  TIM_config_timebase(TIM2, 8400, 10); //interrupt ogni 1ms
  TIM_set(TIM2, 0);
  TIM_on(TIM2);

  for(int i = 0; i < n; i++){
    output[i] = 1;
  }

}

void set_output(){
  output[ind] = 1;
  ind = (ind+1) % n;
  output[ind] = 0;

  GPIO_write(GPIOA, 10, output[0]);
  GPIO_write(GPIOB, 3, output[1]);
  GPIO_write(GPIOB, 5, output[2]);
  GPIO_write(GPIOB, 4, output[3]);
}

void get_input(){

  bool = 0;

  if(!GPIO_read(GPIOB, 10)){
    input = 0;
    bool = 1;

  }
  if(!GPIO_read(GPIOA, 8)) {
    input = 1;
    bool = 1;

  }
  if(!GPIO_read(GPIOA, 9)){
    input = 2;
    bool = 1;

  }
  if(!GPIO_read(GPIOC, 7)){
    input = 3;
    bool = 1;

  }

  if(bool){
  	  last_c = current_c;
  	  current_c = tasti[input][ind];
  	bool_to_print = 1;
  	}
  /*else if( bool == 0 && ind == 3){
  	  last_c = current_c;
  	}*/

}


void loop(){

	if(bool_to_print) {
		bool_to_print= 0;
		printf("Ho premuto: %c \n", current_c);
		last_c = current_c;
	}

}

void TIM2_IRQHandler(void){
  if(TIM_update_check(TIM2)){
    set_output();
    get_input();
    TIM_update_clear(TIM2);
  }
}


int main(){
  setup();
  for(;;){
    loop();
  }
  return 0;
}

