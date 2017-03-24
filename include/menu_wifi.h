#ifndef MENU_WIFI_H
#define MENU_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "wifi.h"

void Menu_WIFI_Init();
void controller_wifi(int btn);
void render_wifi();

#ifdef __cplusplus
}
#endif

#endif
