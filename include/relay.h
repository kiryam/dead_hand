#ifndef RELAY_H
#define RELAY_H

extern int RELAY_STATE;

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"

#define RELAY_OFF 0
#define RELAY_ON 1

void RELAY_Init();
void RELAY_On();
void RELAY_Off();

#ifdef __cplusplus
}
#endif

#endif
