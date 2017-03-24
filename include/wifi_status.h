#ifndef WIFI_STATUS_H
#define WIFI_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "wifi.h"

void WIFI_Status_Init();
void controller_wifi_status(int btn);
void render_wifi_status();

#ifdef __cplusplus
}
#endif

#endif
