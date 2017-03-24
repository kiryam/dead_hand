#include "wifi_status.h"
#include "menu_wifi.h"
#include "wifi.h"
static char ip[40] = {0};
static char mac[40] = {0};
void WIFI_Status_Init(){
	WIFI_Get_Status(&ip, &mac);
}

void WIFI_Status_Unregister(){}

void render_wifi_status(){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,0);

	char ip_str[40] = {0};
	sprintf(ip_str, "IP: %s", ip);
	SSD1306_Puts(ip_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,10);

	char mac_str[40] = {0};
	sprintf(mac_str, "MAC:%s", mac);
	SSD1306_Puts(mac_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,20);

	sprintf(mac_str, "PWD: %s", config_get("password"));
	SSD1306_Puts(mac_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,30);

	SSD1306_Puts("Status: disconnected", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();

};

void controller_wifi_status(int btn){
	if( btn == -1 ){
		WIFI_Status_Unregister();
		return;
	}

	if ( btn & BTN1 ){
		Display_Set_Active_Controller(controller_wifi);
		Display_Set_Active_Render(render_wifi);
		Menu_Wifi_Init();
		return;
	}

	if (btn & BTN4){
		WIFI_Get_Status(&ip, &mac);
	}

}
