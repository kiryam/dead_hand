#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
void Main_Menu_Init();
void render_main_menu();
void controller_main_menu(int btn);

#ifdef __cplusplus
}
#endif

#endif
