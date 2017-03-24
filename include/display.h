#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "ssd1306.h"
#include "fonts.h"
#include "menu.h"
#include "menu_wifi.h"
#include "display_homepage.h"

void Display_Init();
void Display_Render();
void Display_Btn_Pressed(int);
void Display_Set_Active_Controller(int (*controller)(int));
void Display_Set_Active_Render(int (*render)(void));

#ifdef __cplusplus
}
#endif

#endif
