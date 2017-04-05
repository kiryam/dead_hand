#include "wifi_status.h"
#include "menu_wifi.h"
#include "server.h"

static char ip[IP_MAX_LEN] = {0};
static char mac[MAC_MAX_LEN] = {0};
void WIFI_Status_Init(){
	WIFI_Get_Status(&ip, &mac);
}

void WIFI_Status_Unregister(){}

void render_wifi_status(){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,0);

	char ip_str[IP_MAX_LEN+10] = {0};
	sprintf(ip_str, "%s:%d", ip, SERVER_PORT);
	SSD1306_Puts(ip_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,10);

	char mac_str[MAC_MAX_LEN+10] = {0};
	sprintf(mac_str, "MAC:%s", mac);
	SSD1306_Puts(mac_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,20);

	sprintf(mac_str, "PWD: %s", config_get("password"));
	SSD1306_Puts(mac_str, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0,30);

	if( server_status == 0 ){
		SSD1306_Puts("Server: OK", &Font_7x10, SSD1306_COLOR_WHITE);
	}else if (server_status == 1){
		SSD1306_Puts("Server: ERROR", &Font_7x10, SSD1306_COLOR_WHITE);
	}else{
		SSD1306_Puts("Server: STOPED", &Font_7x10, SSD1306_COLOR_WHITE);
	}

	SSD1306_GotoXY(0,40);
	char clients_str[30] = {0};
	sprintf(clients_str, "Clients: %d", clients_count);
	SSD1306_Puts(clients_str, &Font_7x10, SSD1306_COLOR_WHITE);

	SSD1306_GotoXY(0,50);
	char memory_str[40] = {0};
	sprintf(memory_str, "Memory used: %d", get_memory_allocated_total());
	SSD1306_Puts(memory_str, &Font_7x10, SSD1306_COLOR_WHITE);

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
