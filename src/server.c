#include <stdlib.h>
#include <stdio.h>
#include "server.h"
#include "request.h"
#include "message_queue.h"
#include "ipd_parser.h"
#include "handlers.h"


#include "wifi.h"

int server_status = -1;
int clients_count=0;


static int pending_connection[MAX_PENDING_CONNETION];
static int pending_connection_last_item = -1;

static int pending_disconnection[MAX_PENDING_DISCONNECTION];
static int pending_disconnection_last_item = -1;

int open_connections[20] = {0};

typedef struct {
	int connect_id;
	char data[MAX_DATA_SIZE];
} Data_Item;

static Data_Item* pending_data[MAX_PENDING_DATA];
static int pending_data_last_item = -1;

int pending_data_push(Data_Item* item){
	if( pending_data_last_item+1 > MAX_PENDING_DATA) {
		Log_Message("Connection stack overflow");
		return 1;
	}

	pending_data[++pending_data_last_item] = item;
	return 0;
}

Data_Item* pending_data_pop() {
	return pending_data_last_item < 0 ? NULL : pending_data[pending_data_last_item--];
}

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
	open_connections[conn_id] = 1;
	Log_Message("Connected");
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
	NVIC_SetPriority(TIM7_IRQn, 60);
}

void TIM7_IRQHandler() {
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

		int conn_id = pending_disconnection_pop();
		if( conn_id >= 0 ){
			clients_count--;
			open_connections[conn_id] = 0;
			return;
		}

		conn_id = pending_connection_pop();
		if( conn_id >= 0 ){
			func_wifi_server_on_connect(conn_id);
			return;
		}

		ipd_parser* message = (ipd_parser*)message_data_queue_get();

		if( message != NULL ){
			int conn_id = message->conn_id;
			Request request = {0};

			if( request_parse(&request, message->message, message->message_size) != 0){
				Log_Message("Parse header error");
				ipd_parser_free(message);
				return;
			}

			ipd_parser_free(message);

			char response[RESPONSE_MAX_LEN+512] = {0};
			int bytes_pending = handle_request(&request, response);
			request_free(&request);

			char* response_ptr = response;

			char tmp_log[128] = {0};

			while (bytes_pending > 0){
				if( bytes_pending > MAX_PACKET_LEN ){
					if (WIFI_TCP_Send(conn_id, (uint8_t*)response_ptr, MAX_PACKET_LEN) != 0 ){
						bytes_pending=0;
						Log_Message("Send data failed");
					}

					sprintf(tmp_log, "Sent %d bytes  (pending: %d)", MAX_PACKET_LEN, bytes_pending);
					Log_Message(tmp_log);

					response_ptr = &response_ptr[MAX_PACKET_LEN];
					bytes_pending -= MAX_PACKET_LEN;
				} else {
					if (WIFI_TCP_Send(conn_id, (uint8_t*)response_ptr, bytes_pending) != 0 ){
						Log_Message("Send data failed");
					}
					sprintf(tmp_log, "Sent %d bytes", bytes_pending);
					Log_Message(tmp_log);

					bytes_pending=0;
				}
			}
			Log_Message("Send done");
		}
    }
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


int WIFI_Server_Start(int port){
	WIFI_Server_Timer_Init();
	remove_newline_callback((callback)func_wifi_on_new_line);
	add_newline_callback((callback)func_wifi_on_new_line);

	if ( WIFI_Set_CIPSERVER(0, 0) ){
		Log_Message("CIPSERVER=0 ERROR");
		server_status = 1;
		return 1;
	}

	if ( WIFI_Set_CIPMUX(0) ) {
		Log_Message("CIPMUX=0 ERROR");
		server_status = 1;
		return 1;
	}

	if ( WIFI_Set_CIPMODE(0) ) {
		Log_Message("CIPMODE=0 ERROR");
		server_status = 1;
		return 1;
	}

	if ( WIFI_Set_CIPMUX(1) ) {
		Log_Message("CPIMUX=1 ERROR");
		server_status = 1;
		return 1;
	}

	if (WIFI_Set_CIPSERVER(1, port) ) {
		Log_Message("CIPSERVER=1 ERROR");
		server_status = 1;
		return 1;
	}

	Log_Message("Server started");
	server_status = 0;
	return 0;
}

