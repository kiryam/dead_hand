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

#define MEMORY_SIZE 1199523 - 1024


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

	// Write to Backup SRAM with 32-Bit Data
	int i;
	int errorindex=0;
	  // for (i = 0x0; i < 0x100; i += 4) {
		//   *(__IO uint32_t *) (BKPSRAM_BASE + 1) = 10;
	  // }

	   // Check the written Data
	 //  for (i = 0x0; i < 0x100; i += 4) {
	//		  if ((*(__IO uint32_t *) (BKPSRAM_BASE + i)) != i){
	//			  errorindex++;
	//		  }
	 //  }
}

int
main(int argc, char* argv[]) {
	memory_init();
	BCKP_Init();

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	IWDG_SetReload(1000);
	IWDG_ReloadCounter();
	//IWDG_Enable(); // TODO enable on release

	Log_Message("Watchdog ok");



	//FLASH_Test();
	//*(__IO uint32_t *) (BKPSRAM_BASE + 1) = 30;

	int btn_state = 0;
	LED_Init();
	Log_Message("Led ok");
	RELAY_Init();
	Log_Message("Relay ok");
	BTN_Init();
	Log_Message("Btn ok");

	LED_On();
	sleepMs(100);
	LED_Off();
	sleepMs(100);
	LED_On();
	sleepMs(100);
	LED_Off();
	sleepMs(100);
	LED_On();
	sleepMs(100);
	LED_Off();

	Display_Init();
	Log_Message("Display ok");

	WIFI_Init();
	if ( WIFI_Server_Start(SERVER_PORT) == 0 ) {
		Log_Message("Server start ok");
	}else{
		Log_Message("Failed to start server");
	}

	while (1) {
		IWDG_ReloadCounter();


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
		Display_Render();
    }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
