#include "wifi_server.h"
#include "menu_wifi.h"
#include "wifi.h"
#include "request.h"
#include "message_queue.h"
#include "log.h"
#include "config.h"
#include "display.h"

#define FLASH_KEY1      ((uint32_t)0x45670123)
#define FLASH_KEY2      ((uint32_t)0xCDEF89AB)
#define Page_127        0x0801FC00


static char* message = "";
static int server_status = 0;
static uint8_t dots =0;
static int clients_count=0;

typedef struct {
	int connect_id;
} connect_item;

#define MAX_PENDING_CONNETION 10
#define MAX_PENDING_DISCONNECTION 10
#define MAX_PENDING_DATA 10
#define MAX_DATA_SIZE 1024

static int pending_connection[MAX_PENDING_CONNETION];
static int pending_connection_last_item = -1;

static int pending_disconnection[MAX_PENDING_DISCONNECTION];
static int pending_disconnection_last_item = -1;

int pending_connection_push(int conn_id){
	if( pending_connection_last_item+1 > MAX_PENDING_CONNETION) {
		Log_Message("Connection stack overflow");
		return 1;
	}

	pending_connection[++pending_connection_last_item] = conn_id;
	return 0;
}

int pending_connection_pop() {
	return pending_connection_last_item < 0 ? -1 : pending_connection[pending_connection_last_item--];
}

int pending_disconnection_push(int conn_id){
	if( pending_disconnection_last_item+1 > MAX_PENDING_DISCONNECTION ){
		Log_Message("Disconnection stack overflow");
		return 1;
	}

	pending_disconnection[++pending_disconnection_last_item] = conn_id;
	return 0;
}

int pending_disconnection_pop() {
	return pending_disconnection_last_item < 0 ? -1 : pending_disconnection[pending_disconnection_last_item--];
}

void func_wifi_server_on_connect(int conn_id){
	clients_count++;
	Log_Message("Connected");
	//if (WIFI_TCP_Disconnect(conn_id) != 0 ) {
	//	message="Disconnect error";
	//	return;
	//}

	message="Listening";
	return;
}

void WIFI_Server_Timer_Init() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);

	base_timer.TIM_Prescaler = 24000 - 1;
	base_timer.TIM_Period = 100;

	TIM_TimeBaseInit(TIM7, &base_timer);

	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM7, ENABLE);
	NVIC_EnableIRQ(TIM7_IRQn);
}

void TIM7_IRQHandler() {
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

		int conn_id = pending_disconnection_pop();
		if( conn_id >= 0 ){
			clients_count--;
		}

		conn_id = pending_connection_pop();
		if( conn_id >= 0 ){
			func_wifi_server_on_connect(conn_id);
		}


  }
}

void handler_index(Request* request,  uint8_t* response) {
	Log_Message("index");

	char *page = "<html><head><title>Dead hand configure</title></head><body><h1>Set connection</h1><form action=\"/set_passwd\">Password: <input name=\"password\" /><br /><input type=\"submit\" value=\"Connect\" /></form></body></html>";
	strcpy(response, page);
}

void handler_set_password(Request* request, char* response){
	Log_Message("set passwd");
	const char s[2] = "&";
	char *token;
	token = strtok(&request->path[12], s);
	while( token != NULL ) {
		if (strncmp("password", token, 8) == 0){
			config_set("password", &token[9]);
		}
		token = strtok(NULL, s);
	}

	sprintf(response, "Password set to %s", config_get("password"));
}

void handler_get_protocol_log(Request* request, uint8_t* response){
	Log_Message("Get protocol log");
	strncpy(response, get_protocol_log(), RESPONSE_MAX_LEN);
}


void handler_memory(Request* request, uint8_t* response){
	Log_Message("Memory");
	sprintf(response, "Allocated memory: %d of %d", get_memory_allocated_total(), get_memory_max());
}

void func_wifi_on_new_line(char* line){
	if (strncmp(&line[2], "CONNECT", 7) == 0) {
		pending_connection_push(atoi(&line[0]));
	} else if( strncmp(&line[3], "CONNECT", 7) == 0) {
		char ch[3]={0};
		strncpy(ch, line, 2);
		pending_connection_push(atoi(ch));
	} else if (strncmp(&line[2], "CLOSED", 6) == 0) {
		pending_disconnection_push(atoi(&line[0]));
	} else if(strncmp(&line[3], "CLOSED", 6) == 0 ){
		char ch[3]={0};
		strncpy(ch, line, 2);
		pending_disconnection_push(atoi(ch));
	}
}

void func_wifi_ondata(message_data* msg){
	Request* request = request_init(msg->line);

	if( request == NULL ) {
		Log_Message("Request init fail");
		return;
	}

	uint8_t response[RESPONSE_MAX_LEN] = {0};
	if( strncmp("/set_passwd", request->path, 11) == 0 ){
		handler_set_password(&request, response);
	} else if(strncmp("/get_protocol_log", request->path, 18) ==0){
		handler_get_protocol_log(&request, response);
	} else if(strncmp("/memory", request->path, 7) ==0){
		handler_memory(&request, response);
	} else {
		handler_index(&request, response);
	}

	request_free(request);

	int resonse_len = strlen(response);
	uint8_t welcome[512] = {0};

	sprintf(welcome, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\n\r\n", resonse_len);

	if (WIFI_TCP_Send(msg->target, welcome, strlen(welcome)) != 0 ){
		message="Failed to send";
		return;
	}

	int bytes_pending = resonse_len;
	uint8_t* response_ptr = response;

#define MAX_PACKET_LEN 256

	uint8_t tmp_buff[MAX_PACKET_LEN+1] ={0};
	while (bytes_pending){
		if( bytes_pending > MAX_PACKET_LEN ){
			strncpy(tmp_buff, response_ptr, MAX_PACKET_LEN);
			if (WIFI_TCP_Send(msg->target, tmp_buff, MAX_PACKET_LEN) != 0 ){
				message="Failed to send";
			}
			response_ptr = &response_ptr[MAX_PACKET_LEN];
			bytes_pending -= MAX_PACKET_LEN;
		} else {
			if (WIFI_TCP_Send(msg->target, response_ptr, bytes_pending) != 0 ){
				message="Failed to send";
			}
			bytes_pending=0;
		}
	}
}

void WIFI_Server_Init(){
	WIFI_Server_Timer_Init();
	//if( WIFI_Disconnect() != 0 ){
	//	message="Filed to disconnect";
	//	return;
	//}

	if ( WIFI_Set_CIPMUX(0) ) {
		Log_Message("CIPMUX=0 ERROR");
		return;
	}
	Log_Message("CIPMUX=0 OK");

	if (WIFI_Set_CIPSERVER(0, -1) ) {
		Log_Message("CIPSERVER=0 ERROR");
		return;
	}
	Log_Message("CIPSERVER=0 OK");

	if (WIFI_Set_CWMODE(2) != 0 ) {
		message="Failed to set CWMODE";
		return;
	}
	Log_Message("CWMODE=2 OK");

	if (WIFI_Set_CWSAP("Dead_Hand", "", 5, 0) != 0){
		message="Failed to run AP";
		return;
	}

	message = "";
	if ( WIFI_Server_Start(8888) == 0 ) {
		add_newline_callback(func_wifi_on_new_line);
		add_data_callback(func_wifi_ondata);

		server_status = 1;
		message = "Listening";
	}else {
		message = "Failed to create server";
	}
}

void WIFI_Server_Unregister(){
	remove_newline_callback(func_wifi_on_new_line);
	remove_data_callback(func_wifi_ondata);
}

void render_wifi_server(){
	if (++dots>4){dots=0;}

	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,0);

	SSD1306_Puts(message, &Font_7x10, SSD1306_COLOR_WHITE);
	for(uint8_t i=0; i<=dots;i++){
		SSD1306_Puts(".", &Font_7x10, SSD1306_COLOR_WHITE);
	}

	SSD1306_GotoXY(0,12);
	char clients_count_msg[17] = "Clients count: ";
	itoa(clients_count, &clients_count_msg[15], 10);
	SSD1306_Puts(clients_count_msg, &Font_7x10, SSD1306_COLOR_WHITE);

	SSD1306_UpdateScreen();
};

void controller_wifi_server(int btn){
	if( btn == -1 ){
		WIFI_Server_Unregister();
		return;
	}

	if ( btn & BTN1 ){
		Display_Set_Active_Controller(controller_wifi);
		Display_Set_Active_Render(render_wifi);
		Menu_Wifi_Init();
		return;
	}
}
