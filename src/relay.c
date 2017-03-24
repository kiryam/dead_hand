#include "relay.h";

int RELAY_STATE;

void RELAY_Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef gpio_port;

	gpio_port.GPIO_Pin   = GPIO_Pin_1;
	gpio_port.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_port.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpio_port.GPIO_OType = GPIO_OType_PP;
	gpio_port.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpio_port);
}

void RELAY_On() {
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	RELAY_STATE = RELAY_ON;
}

void RELAY_Off() {
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	RELAY_STATE = RELAY_OFF;
}
