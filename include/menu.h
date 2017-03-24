#ifndef MENU_H
#define MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "common.h"
#include "display.h"
#include "ssd1306.h"

typedef struct {
	char caption[20];
	char param1[20];
	int is_selected;
	functiontype function;
} MENU_List_Item;


typedef struct {
	int count;
	MENU_List_Item* items;
} List_Elements;

void Menu_Init();
void Menu_Free();
void Menu_Add_Item( char* caption,  functiontype fnc);
void Menu_Add_Item_Param( char* caption,  functiontype fnc, char* param1);
MENU_List_Item* Menu_Get_Active_Item();

#ifdef __cplusplus
}
#endif

#endif
