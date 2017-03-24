#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

extern int RELAY_STATE;

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "sds.h"

void WIFI_Connect_Init();
void controller_wifi_connect(int btn);
void render_wifi_connect();

#ifdef __cplusplus
}
#endif

#endif
