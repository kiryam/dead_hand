#include <stdio.h>
#include <stdlib.h>

#include "led.h"
#include "wifi.h"
#include "common.h"
#include "relay.h"
#include "display.h"
#include <stdlib.h>
#include "log.h"
#include "server.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


#ifdef __cplusplus
extern "C" {
#endif


void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		Display_Render();
	}
}


#ifdef __cplusplus
}
#endif

void BTN_Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure_Btn;
	GPIO_InitStructure_Btn.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure_Btn.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure_Btn.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure_Btn.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure_Btn);
}

void BCKP_Init(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	PWR_BackupRegulatorCmd(ENABLE);
}

void NVIC_Init(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure_USART1;
	NVIC_InitStructure_USART1.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure_USART1.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure_USART1.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure_USART1.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_USART1);

	NVIC_InitTypeDef NVIC_InitStructure_TIM6;
	NVIC_InitStructure_TIM6.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure_TIM6.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure_TIM6.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure_TIM6.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_TIM6);

	NVIC_InitTypeDef NVIC_InitStructure_TIM7;
	NVIC_InitStructure_TIM7.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure_TIM7.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure_TIM7.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure_TIM7.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_TIM7);

	NVIC_InitTypeDef NVIC_InitStructure_TIM2;
	NVIC_InitStructure_TIM2.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_TIM2);
}

void RenderTimer_Init(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);


	base_timer.TIM_Prescaler = 24000 - 1;
	base_timer.TIM_Period = 1000/1;
	TIM_TimeBaseInit(TIM2, &base_timer);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

int main(int argc, char* argv[]) {
	SysTick_Init();
	NVIC_Init();
	memory_init();
	BCKP_Init();

#ifdef WATCHDOG_ENABLED
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	IWDG_SetReload(1000);
	IWDG_ReloadCounter();
	IWDG_Enable(); // TODO enable on release
	Log_Message("Watchdog ok");
#else
	Log_Message("Watchdog disabled");
#endif

	int btn_state = 0;
	LED_Init();
	Log_Message("Led ok");
	RELAY_Init();
	Log_Message("Relay ok");
	BTN_Init();
	Log_Message("Btn ok");

	LED_On();
	sleepMs(500);
	LED_Off();
	sleepMs(500);
	LED_On();
	sleepMs(500);
	LED_Off();
	sleepMs(500);
	LED_On();
	sleepMs(500);
	LED_Off();

	Display_Init();
	Log_Message("Display ok");

	RenderTimer_Init();



	WIFI_Init();
	if ( WIFI_Server_Start(SERVER_PORT) == 0 ) {
		Log_Message("Server start ok");
	}else{
		Log_Message("Failed to start server");
	}



	while (1) {
#ifdef WATCHDOG_ENABLED
		IWDG_ReloadCounter();
#endif
		uint8_t btn_pressed = 0;

		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) ){
			if ((btn_state & BTN4) == 0 ){
				btn_state |= BTN4;
				btn_pressed |= BTN4;
			}
		}else{
			btn_state = btn_state & ~BTN4;
		}

		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) ){
			if ((btn_state & BTN3) == 0 ){
				btn_state |= BTN3;
				btn_pressed |= BTN3;
			}
		}else{
			btn_state = btn_state & ~BTN3;
		}

		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) ){
			if ((btn_state & BTN2) == 0 ){
				btn_state |= BTN2;
				btn_pressed |= BTN2;
			}
		}else{
			btn_state = btn_state & ~BTN2;
		}

		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) ){
			if ((btn_state & BTN1) == 0 ){
				btn_state |= BTN1;
				btn_pressed |= BTN1;
			}
		}else{
			btn_state = btn_state & ~BTN1;
		}

		Display_Btn_Pressed(btn_pressed);
		sleepMs(100);
    }

}

#pragma GCC diagnostic pop
