#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "wifi.h"


void WIFI_Server_Init();
void controller_wifi_server(int btn);
void render_wifi_server();

#ifdef __cplusplus
}
#endif

#endif
