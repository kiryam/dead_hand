#ifndef WIFI_H
#define WIFI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f2xx.h"

#include "ssd1306.h"
#include "common.h"

#define ESP_DISABLE_ECHO

#define MAX_NEWLINE_CALLBACK_COUNT 4
#define MAX_DATA_CALLBACK_COUNT 5

#define MAX_POINTS_COUNT 50
#define WIFI_POINT_MAX_LEN 60

#define IP_MAX_LEN 40
#define MAC_MAX_LEN 41


extern int wifi_init_ok;
typedef struct {
	char name[WIFI_POINT_MAX_LEN];
} WIFI_Point;

typedef struct {
	int found;
	WIFI_Point* points[MAX_POINTS_COUNT];
} WIFI_List_Result;

void MY_USART_Init();
int WIFI_Init();
int WIFI_Test();
int WIFI_Connect(char* wifi_name, char* wifi_pass);
int WIFI_Retreive_List();
int WIFI_Get_Status();
int WIFI_Disconnect();
int WIFI_TCP_Connect(char* host, unsigned int port);
int WIFI_TCP_Disconnect(uint8_t conn_id);
int WIFI_TCP_Send(uint8_t conn_id, uint8_t* data, unsigned int bytes_count);
int WIFI_TCP_Recv(uint8_t* response);
int WIFI_Read_Char(char* ch, int timeout);
int WIFI_Server_Start(int port);
int WIFI_Set_CIPMODE(int mode);
int WIFI_Set_CWMODE(int mode);
int WIFI_Set_CIPMUX(int mode);
int WIFI_Set_CIPSERVER(int mode, int port);
int WIFI_Set_CWSAP(char* ssid, char* password, uint8_t channel, uint8_t ecn);
int WIFI_Read_Byte(uint16_t* ch, int timeout);
int WIFI_Exec_Cmd_Get_Answer(char* cmd, char* answer);
int WIFI_Read_Line(char* answer,size_t maxlen, int unsigned timeout_ms);
void WIFI_Send_Bytes(uint8_t* bytes, unsigned int bytes_count, unsigned int timeout_ms);
extern int is_new_line;

int add_newline_callback(callback f);
int remove_newline_callback(callback f);

#ifdef __cplusplus
}
#endif

#endif
