#ifndef WIFI_H
#define WIFI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f2xx.h"

#include "ssd1306.h"
#include "common.h"

#define MAX_NEWLINE_CALLBACK_COUNT 10
#define WIFI_BUFF_SIZE 2048
#define WIFI_LINE_BUFF_SIZE 2048

typedef struct {
	char name[20];
} WIFI_Point;

#define MAX_POINTS_COUNT 5


void WIFI_Init();
int WIFI_Test();
int WIFI_Connect(char* wifi_name, char* wifi_pass);
int WIFI_Retreive_List();
int WIFI_Get_Status();
int WIFI_Disconnect();
int WIFI_TCP_Connect(char* host, int port);
int WIFI_TCP_Disconnect(uint8_t conn_id);
int WIFI_TCP_Send(uint8_t conn_id, uint16_t* data, int bytes_count);
int WIFI_TCP_Recv(uint8_t* response);
int WIFI_Read_Char(char* ch, int timeout);
int WIFI_Server_Start(int port);
int WIFI_Set_CIPMODE(int mode);
int WIFI_Set_CWMODE(int mode);
int WIFI_Set_CIPMUX(int mode);
int WIFI_Set_CIPSERVER(int mode, int port);
int WIFI_Set_CWSAP(char* ssid, char* password, uint8_t channel, uint8_t ecn);

extern int is_new_line;


#ifdef __cplusplus
}
#endif

#endif
