#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"

void LED_Init();
void LED_On();
void LED_Off();

#ifdef __cplusplus
}
#endif

#endif
