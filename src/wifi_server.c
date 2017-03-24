#include "wifi_server.h"
#include "menu_wifi.h"

#define FLASH_KEY1      ((uint32_t)0x45670123)
#define FLASH_KEY2      ((uint32_t)0xCDEF89AB)
#define Page_127        0x0801FC00


static char* message = "";
static int server_status = 0;
static uint8_t dots =0;
static char buff[1024] = {0};
static int clients_count=0;

static int new_client = -1;

/*
void WIFI_Server_Timer_Init() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);

	base_timer.TIM_Prescaler = 24000 - 1;
	base_timer.TIM_Period = 500;
	TIM_TimeBaseInit(TIM6, &base_timer);

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM6, ENABLE);

	NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void TIM6_DAC_IRQHandler() {
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		//GPIO_Write(GPIOC, GPIO_ReadOutputData(GPIOC) ^ (BLUE_LED | GREEN_LED));
  }
}
*/

void fun_wifi_on_new_line(char* line){ // TODO not void
	if (strncmp(&line[2], "CONNECT", 7) == 0 || strncmp(&line[3], "CONNECT", 7) == 0){
		char answer[2048] = {0};
		if ( WIFI_Read_Line_Sync(answer, 100, 800000) == 0){
			for(int i=0; i<5;i++){
				if( WIFI_Read_Byte(&answer[i], 200000) ){
					// TODO ERROR Handling
				}
			}

			if( strncmp("+IPD", answer, 4) == 0 ){
				int meta_estimated = 10;
				int conn_id = 0;
				for( int i=0; i<meta_estimated; i++){
					WIFI_Read_Byte(&answer[i], 400000);
					if( answer[i] == ',' ){
						answer[i] = '\0';
						conn_id = atoi(answer);
						i=-1;
					}

					if( answer[i] == ':' ){
						answer[i] = '\0';
						meta_estimated = 0;
					}
				}

				int size_estimated = atoi(answer);
				for(int i=0; i<size_estimated;i++){
					WIFI_Read_Byte(&answer[i], 400000);
				}


				if (strncmp("GET /?password", answer, 14) == 0){
					char param_str[512] = {0};
					for(int i=0; i<sizeof(param_str); i++) {
						if(answer[6+i] == ' '){
							break;
						}
						param_str[i]=answer[6+i];
					}

					const char s[2] = "&";
					char *token;
					token = strtok(param_str, s);
					while( token != NULL ) {
						if (strncmp("password", token, 8) == 0){
							config_set("password", &token[9]);
						}
						token = strtok(NULL, s);
					}
				}

				char* password = config_get("password");
				message="IPD OK";
				new_client = conn_id;
			}
		}

	} else if (strncmp(&line[2], "CLOSED", 6) == 0 || strncmp(&line[3], "CLOSED", 6) == 0){
		message="closed";
		clients_count--;
	}else{
		strcpy(message, line);
		//message = "err";
	}
}

void func_wifi_server_on_connect(int conn_id){
	clients_count++;
	message = "Client connected";
	Log_Message("Connected");

	char *page = "<html><head><title>Dead hand configure</title></head><body><h1>Set connection</h1><form>Password: <input name=\"password\" /><br /><input type=\"submit\" value=\"Connect\" /></form></body></html>";
	char welcome[1024]="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	itoa(strlen(page), &welcome[strlen(welcome)], 10);
	strcat(welcome, "\r\nConnection: Closed\r\n\n");
	strcat(welcome, page);

	if (WIFI_TCP_Send(conn_id, welcome, strlen(welcome)) != 0 ){
		message="Failed to send";
		return;
	}

	//if (WIFI_TCP_Disconnect(conn_id) != 0 ) {
	//	message="Disconnect error";
	//	return;
	//}

	message="Listening";
	return;
}

void func_wifi_server_closed(int conn_id){
	message = "Listening";
	return;
}

void func_wifi_server_on_data(){
	buff[0]= '\0';
	// TODO pass connection ID and data here
}

void WIFI_Server_Init(){
	//WIFI_Server_Time_Init();
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
		add_newline_callback(fun_wifi_on_new_line);

		server_status = 1;
		message = "Listening";
	}else {
		message = "Failed to create server";
	}
}

void WIFI_Server_Unregister(){
	remove_newline_callback(fun_wifi_on_new_line);
}

void render_wifi_server(){
	if( new_client != -1 ) {
		int client_id = new_client;
		new_client = -1;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		func_wifi_server_on_connect(client_id);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	}

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


	//SSD1306_GotoXY(0,24);
	//SSD1306_Puts(server_buff, &Font_7x10, SSD1306_COLOR_WHITE);

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
