#include "menu_wifi.h"
#include "menu.h"
#include "main_menu.h"
#include "wifi_status.h"
#include "wifi_connect.h"
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <stdbool.h>
#include "server.h"

const unsigned char img24x24 [] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

const unsigned char wifi [] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x02, 0x00,
0x00, 0x00, 0x00, 0x40,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x00, 0x00,
0x00, 0x02, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
};

void func_wifi_status() {
	Display_Set_Active_Controller(controller_wifi_status);
	Display_Set_Active_Render(render_wifi_status);
	WIFI_Status_Init();
}

void func_wifi_connect() {
	Display_Set_Active_Controller(controller_wifi_connect);
	Display_Set_Active_Render(render_wifi_connect);
	WIFI_Connect_Init();
}

void func_wifi_disconnect() {
	WIFI_Disconnect();
	Display_Set_Active_Controller(controller_wifi_status);
	Display_Set_Active_Render(render_wifi_status);
	WIFI_Status_Init();
}

void func_wifi_server(){
	if( server_status != 1 ){
		if ( WIFI_Server_Start(SERVER_PORT) == 0 ) {
			Log_Message("Server start ok");
		}else{
			Log_Message("Failed to start server");
			return;
		}
	}
	Display_Set_Active_Controller(controller_wifi_status);
	Display_Set_Active_Render(render_wifi_status);

	WIFI_Status_Init();
}


void func_start_ap(){
	if (WIFI_Set_CIPSERVER(0, -1) ) {
		Log_Message("CIPSERVER=0 ERROR");
		//return;
	}

	if ( WIFI_Set_CIPMUX(0) ) {
		Log_Message("CIPMUX=0 ERROR");
		//return;
	}
	Log_Message("CIPMUX=0 OK");

	if (WIFI_Set_CIPSERVER(0, -1) ) {
		Log_Message("CIPSERVER=0 ERROR");
		//return;
	}
	Log_Message("CIPSERVER=0 OK");

	if (WIFI_Set_CWMODE(3) != 0 ) {
		Log_Message("CWMODE ERROR");
		return;
	}
	Log_Message("CWMODE=3 OK");

	if (WIFI_Set_CWSAP("Dead_Hand", "", 5, 0) != 0){
		Log_Message("AP point start err");
		//message="Failed to run AP";
		return;
	}

	Display_Set_Active_Controller(controller_wifi_status);
	Display_Set_Active_Render(render_wifi_status);
	WIFI_Status_Init();
}


void func_wifi_tcp_connect() {
	// FIXME
	int conn_ok = 0;
	uint8_t answer[100] = {0};
	WIFI_TCP_Connect("192.168.1.216", 7000); // TODO
	WIFI_TCP_Send(0, "0",1);

	//int bytes_read = WIFI_TCP_Recv(answer); // TODO CHECK

	/*if ( bytes_read >= 0){
		Hello hello_message = {42};
		pb_istream_t stream = pb_istream_from_buffer(answer, bytes_read);

		if (pb_decode( &stream, Hello_fields, &hello_message) == true ){
			Log_Message("Hello message ok");
			conn_ok=1;
		}
		conn_ok=0;
	}
	*/

	WIFI_TCP_Disconnect(0);
}


void Menu_Wifi_Init(){
	Menu_Init();
	Menu_Add_Item("Status", func_wifi_status);
	Menu_Add_Item("Connect", func_wifi_connect);
	Menu_Add_Item("Disconnect", func_wifi_disconnect);
	Menu_Add_Item("Connect TCP", func_wifi_tcp_connect);
	Menu_Add_Item("Start AP", func_start_ap);
	Menu_Add_Item("Server Start", func_wifi_server);
}

void Menu_WIFI_Unregister(){
	Menu_Free();
}

void controller_wifi(int btn){
	if( btn == -1 ){
		Menu_WIFI_Unregister();
		return;
	}
	if ( btn & BTN1 ){
		Display_Set_Active_Controller(controller_main_menu);
		Display_Set_Active_Render(render_main_menu);
		Main_Menu_Init();
		return;
	}

	controller_menu(btn);
}

void render_wifi(){
	render_menu();
}
