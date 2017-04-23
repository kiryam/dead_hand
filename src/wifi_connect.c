#include "wifi_connect.h"
#include "display.h"
#include "wifi_status.h"
#include "wifi.h"

static int point_list_err = 0;

void func_connect(){
	MENU_List_Item* active_item = Menu_Get_Active_Item();
	if ( WIFI_Disconnect() != 0 ) {
		Log_Message("Disconnect error");
		return;
	}

	Log_Message("Disconnected from AP");
	char msg[200] = {0};
	sprintf("Trying to connect: %s - %s", active_item->param1, config_get("password"));
	Log_Message(msg);

	if ( WIFI_Connect(active_item->param1, config_get("password")) == 0 ){
		Log_Message("Connected");
		Display_Set_Active_Controller(controller_wifi_status);
		Display_Set_Active_Render(render_wifi_status);
		WIFI_Status_Init();
	}
}

void retreive_point_list(){
	WIFI_List_Result* result = malloc_c(sizeof(WIFI_List_Result));
	if(result == NULL){
		Log_Message("Out of memory");
		return;
	}
	point_list_err = WIFI_Retreive_List(result);

	if( point_list_err == 0 ){
		for(int i=0; i<result->found;i++){
			WIFI_Point* point = result->points[i];
			Menu_Add_Item_Param(point->name, func_connect, point->name);
		}
	}

	WIFI_List_Result_Free(result);
}

void WIFI_Connect_Init() {
	Menu_Init();

	char* name = config_get("name");
	if( name[0] != '\0'){
		Menu_Add_Item_Param(name, func_connect, config_get("password"));
	}

	retreive_point_list();
}

void WIFI_Connect_Unregister(){
	//remove_newline_callback(func_parse_point);
	Menu_Free();
}

void render_point_list() {
	if(point_list_err != 0) {
		SSD1306_Puts("Point list err", &Font_7x10, SSD1306_COLOR_WHITE);
	}else{
		render_menu();
	}
	/*SSD1306_DrawFilledRectangle(0, SSD1306_GetY(), 128, 12, SSD1306_COLOR_WHITE);
	SSD1306_MoveRelative(0, 2);
	SSD1306_Puts("AVAILABLE NETWORKS", &Font_7x10, SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,SSD1306_GetY()+12);

	for(int i=0;i<points_Info.count;i++){
		SSD1306_Puts(points_Info.points[i].name, &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_MoveRelative(0, 12);
		//SSD1306_GotoXY(0,18+10*i+2);
	}
	*/

}

void render_wifi_connect(){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,0);
	render_point_list();
	SSD1306_UpdateScreen();
}

void controller_wifi_connect(int btn){
	if( btn == -1 ){
		WIFI_Connect_Unregister();
		return;
	}
	if ( btn & BTN1 ){
		Display_Set_Active_Controller(controller_wifi);
		Display_Set_Active_Render(render_wifi);
		Menu_Wifi_Init();
		return;
	}

	controller_menu(btn);
}
