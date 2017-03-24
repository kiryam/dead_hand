#include "main_menu.h"
#include "menu.h"
#include "ssd1306.h"
#include "log.h"

void func_wifi_menu() {
	Display_Set_Active_Controller(controller_wifi);
	Display_Set_Active_Render(render_wifi);
	Menu_Wifi_Init();
}

void func_relay_on(){
	RELAY_Off();
	Display_Set_Active_Controller(controller_homepage);
	Display_Set_Active_Render(render_homepage);
}

void func_relay_off(){
	RELAY_On();
	Display_Set_Active_Controller(controller_homepage);
	Display_Set_Active_Render(render_homepage);
}

void func_logs(){
	Log_Init();
	Display_Set_Active_Controller(controller_log);
	Display_Set_Active_Render(render_log);
}


void Main_Menu_Init(){
	Menu_Init();
	Menu_Add_Item("WIFI", func_wifi_menu);
	Menu_Add_Item("Relay On", func_relay_on);
	Menu_Add_Item("Relay Off", func_relay_off);
	Menu_Add_Item("Reset", NVIC_SystemReset);
	Menu_Add_Item("Logs", func_logs);
}

void render_main_menu() {
	render_menu();
}

void Main_Menu_Unregister(){
	Menu_Free();
}

void controller_main_menu(int btn) {
	if( btn == -1 ){
		Main_Menu_Unregister();
		return;
	}
	if (btn & BTN1) {
		Display_Set_Active_Controller(controller_homepage);
		Display_Set_Active_Render(render_homepage);
		return;
	}

	controller_menu(btn);
}
