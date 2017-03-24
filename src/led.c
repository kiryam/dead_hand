#include "led.h"

#ifdef __cplusplus
extern "C" {
#endif

void LED_Init(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure_Led;
	GPIO_InitStructure_Led.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure_Led.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure_Led.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure_Led.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure_Led.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure_Led);
}

void LED_On() {
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void LED_Off() {
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}


#ifdef __cplusplus
}
#endif
