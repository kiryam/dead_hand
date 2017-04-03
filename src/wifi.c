#include <stdio.h>
#include "wifi.h"
#include "log.h"
#include "message_queue.h"


int wifi_init_ok = 0;
static char wifi_buff[MESSAGE_COMMAND_SIZE] = {0};
static int wifi_buff_pos=0;
int is_new_line=0;
int is_welcome_byte=0;

static callback new_line_handlers[MAX_NEWLINE_CALLBACK_COUNT] = {0};
static callback data_handlers[MAX_DATA_CALLBACK_COUNT] = {0};

static int recv_message_data_started=0;

static int recv_message_conn_id_readed=0;
static char recv_message_conn_id_tmp[4] = {0};
static int recv_message_conn_id =-1;
static int recv_message_size=-1;
static char recv_message_size_tmp[8] = {0};
static int recv_message_size_cur_pos=0;
static int recv_message_size_readed=0;
static int recv_message_header_readed=0;
static int recv_message_data_bytes_read=0;
static char recv_message_buff[MESSAGE_DATA_MAX_SIZE] = {0};
static int packed_bytes_readed=0;
static int recv_message_conn_id_cur_pos=0;

#define PROTOCOL_LOG_MAX_LENGTH 1024*3
char protocol_log[PROTOCOL_LOG_MAX_LENGTH] = {0};
static int protocol_cursor=0;

enum PROTOCOL_LOG_DIR {DIR_IN, DIR_OUT};
void protocol_log_byte(uint16_t byte, uint8_t dir){
	if( protocol_cursor >= (PROTOCOL_LOG_MAX_LENGTH-10) ){
		protocol_cursor=0;
		protocol_log[0] = '\0';
	}
	protocol_log[protocol_cursor++] = byte;
}

char* get_protocol_log(){
	return protocol_log;
}

int add_newline_callback(callback f){
	for(int i=0; i<MAX_NEWLINE_CALLBACK_COUNT; i++){
		if (new_line_handlers[i] == NULL){
			new_line_handlers[i] = f;
			return 0;
		}
	}

	return 1;
}

int remove_newline_callback(callback f){
	for(int i=0; i<MAX_NEWLINE_CALLBACK_COUNT; i++){
		if (new_line_handlers[i] == f){
			new_line_handlers[i] = NULL;
			return 0;
		}
	}

	return 1;
}

int add_data_callback(callback f){
	for(int i=0; i<MAX_DATA_CALLBACK_COUNT; i++){
		if (data_handlers[i] == NULL){
			data_handlers[i] = f;
			return 0;
		}
	}

	return 1;
}

int remove_data_callback(callback f){
	for(int i=0; i<MAX_DATA_CALLBACK_COUNT; i++){
		if (data_handlers[i] == f){
			data_handlers[i] = NULL;
			return 0;
		}
	}

	return 1;
}

int wait_welcome_byte(int timeout){
	while ( timeout > 0){
		if (is_welcome_byte ){
			return 0;
		}
		timeout--;
	}
	return is_welcome_byte ? 0 : 1;
}

int wait_new_line(int timeout){
	while ( timeout > 0){
		if (is_new_line ){
			return 0;
		}
		timeout--;
	}
	return is_new_line ? 0 : 1;
}

void TIM6_DAC_IRQHandler() {
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		message_command* msg = message_queue_get();
		if( msg != NULL ){
			for(int i=0; i<MAX_NEWLINE_CALLBACK_COUNT; i++){
				if (new_line_handlers[i] != NULL){
					new_line_handlers[i](&msg->line);
				}
			}
			free_c(msg);
		}

		message_data* msg_data = message_data_queue_get();
		if( msg_data ){
			for(int i=0; i<MAX_DATA_CALLBACK_COUNT; i++){
				if (data_handlers[i] != NULL){
					data_handlers[i](msg_data);
				}
			}
			free_c(msg_data);
		}
  }
}

int parse_ipd_packet(char cur_byte) {
	if (!recv_message_data_started) {
		return 0;
	}

	packed_bytes_readed++;

	if( recv_message_conn_id_readed==0){ //IPD,10,
		if(  packed_bytes_readed > 5 ) {
			recv_message_data_started = 0;
			Log_Message("Failed to read +IPD packet");
			return 1;
		}

		if(cur_byte == ','){
			recv_message_conn_id = atoi(recv_message_conn_id_tmp);
			recv_message_conn_id_readed = 1;
		} else {
			recv_message_conn_id_tmp[recv_message_conn_id_cur_pos++] = cur_byte;
		}

		return 1;
	}

	if ( recv_message_conn_id_readed && recv_message_size_readed == 0 ){ //IPD,10,1024:
		if(  packed_bytes_readed > 10 ) {
			recv_message_data_started = 0;
			Log_Message("Failed to read +IPD packet");
			return 1;
		}
		if(cur_byte == ':'){
			recv_message_size = atoi(recv_message_size_tmp);
			if(recv_message_size >= MESSAGE_DATA_MAX_SIZE){
				recv_message_data_started = 0;
				Log_Message("Message to long");
				return 1;
			}else {
				recv_message_size_readed = 1;
			}
		} else {
			recv_message_size_tmp[recv_message_size_cur_pos++] = cur_byte;
		}

		return 1;
	}

	if ( recv_message_conn_id_readed && recv_message_size_readed && packed_bytes_readed ){ //IPD,10,1024:[A-Z]{MESSAGE_DATA_MAX_SIZE}
		if(  packed_bytes_readed > (MESSAGE_DATA_MAX_SIZE+10) ) {
			recv_message_data_started = 0;
			Log_Message("Failed to read +IPD packet");
			return 1;
		}

		if ( recv_message_data_bytes_read < (recv_message_size-1)  ){
			recv_message_buff[recv_message_data_bytes_read++] = cur_byte;
		} else {
			recv_message_data_started = 0;
			message_data_queue_add(recv_message_conn_id, recv_message_buff);
			Log_Message(recv_message_buff);
			wifi_buff_pos=0;
			return 0;
			// message fully readed continue parse string
		}

		return 1;
	}

	recv_message_data_started=0;
	Log_Message("Failed to read +IPD packet");
	return 1;
}

void USART1_IRQHandler(void) {
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		wifi_buff[wifi_buff_pos] = USART_ReceiveData(USART1);

		protocol_log_byte(wifi_buff[wifi_buff_pos], DIR_IN);

		if( wifi_buff_pos == 0 && wifi_buff[wifi_buff_pos] == '>' ){
			is_welcome_byte = 1;
		} else if( parse_ipd_packet(wifi_buff[wifi_buff_pos] ) ) {
			wifi_buff_pos=0;
		} else if(wifi_buff[wifi_buff_pos] == '\n' ){
			is_new_line = 1;
			wifi_buff[wifi_buff_pos+1] = '\0';
			message_queue_add(wifi_buff);
			wifi_buff_pos=0;
		} else {
			wifi_buff_pos++;
		}

		if ( wifi_buff_pos >= MESSAGE_COMMAND_SIZE ){
			wifi_buff_pos = 0;
		}

		if( wifi_buff[0] == '+' && strncmp(wifi_buff, "+IPD,", 5) == 0 ){
			if( recv_message_data_started == 0) {
				recv_message_data_started=1;
				recv_message_conn_id_readed=0;
				recv_message_header_readed=0;
				recv_message_size_readed = 0;
				recv_message_data_bytes_read =0;
				recv_message_size_cur_pos=0;
				recv_message_conn_id_cur_pos=0;
				packed_bytes_readed=0;
			}
		}
	}
}

void MY_USART_Init(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef gpio_port;
	gpio_port.GPIO_Pin   = GPIO_Pin_10;
	gpio_port.GPIO_Mode  = GPIO_Mode_AF;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_port.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(GPIOA, &gpio_port);

	GPIO_InitTypeDef gpio_port1;
	gpio_port1.GPIO_Pin   = GPIO_Pin_9;
	gpio_port1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port1.GPIO_Mode  = GPIO_Mode_AF;
	gpio_port1.GPIO_PuPd = GPIO_PuPd_UP;
	gpio_port1.GPIO_OType =  GPIO_OType_PP;
	GPIO_Init(GPIOA, &gpio_port1);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	USART_InitTypeDef usart;
	usart.USART_BaudRate = 115200;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;

	USART_Init(USART1, &usart);
	USART_Cmd(USART1, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);

	base_timer.TIM_Prescaler = 24000 - 1;
	base_timer.TIM_Period = 20;
	TIM_TimeBaseInit(TIM6, &base_timer);

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM6, ENABLE);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM6_DAC_IRQn, 54);
}

// TODO REMOVE
// DEPRECATED
void WIFI_Send_Command(char* command, uint8_t timeout){
	//Log_Message(command);
	// TODO implement timeout
	for (unsigned int i=0; i<strlen(command); i++){
		USART_SendData(USART1, command[i]);
		protocol_log_byte(command[i], DIR_OUT);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
	}
}

void WIFI_Send_Bytes(uint8_t* bytes, unsigned int bytes_count,  uint8_t timeout){
	// TODO implement timeout
	for (unsigned int i=0; i<=bytes_count; i++){
		USART_SendData(USART1, bytes[i]);
		//protocol_log_byte((char)bytes[i], DIR_OUT);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
	}
}

int WIFI_Read_Line(char* answer,size_t maxlen, int timeout){
	if (wait_new_line(timeout) == 0) {
		strncpy(answer, wifi_buff, maxlen);
		is_new_line=0;
		return 0;
	}
	return 1;
}

int WIFI_Exec_Cmd_Get_Answer(char* cmd, char* answer){
	is_new_line=0;
	WIFI_Send_Command(cmd, 0);
	if (WIFI_Read_Line(answer, 100, 800000) != 0 ){
		return 1;
	}

#ifndef ESP_DISABLE_ECHO
	if (WIFI_Read_Line(answer,100, 8000000) != 0){
		return 1;
	}
#endif

	return WIFI_Read_Line(answer,100, 800000);
}

int WIFI_Reset(){
	char answer[100] = {0};
	WIFI_Exec_Cmd_Get_Answer("AT+RST\r\n", answer);

	return strcmp("OK\r\n\r", answer);
}

int WIFI_Test(){
	char answer[100] = {0};
	if (WIFI_Exec_Cmd_Get_Answer("AT\r\n", answer) != 0) {
		return 1;
	}

	if(strncmp("\r\n", answer, 2) ==0){
		WIFI_Read_Line(answer,100, 200000);
	}
	return strncmp("OK", answer, 2);
}

int WIFI_Set_CIPMODE(int mode){
	char answer[100] = {0};
	char command[19]= "AT+CIPMODE=";
	itoa(mode, &command[strlen(command)], 10);
	strcat(command, "\r\n");

	is_new_line=0;
	WIFI_Send_Command(command, 0);
	WIFI_Read_Line(answer,100, 200000);

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer,100, 200000);
#endif

	if (strncmp("CIPMUX and CIPSERVER must be 0", answer, 30) == 0){
		Log_Message("CIPMUX and CIPSERVER must be 0");
		return 1;
	}
	WIFI_Read_Line(answer,100, 200000);

	return strncmp(answer, "OK", 2);
}

int WIFI_Set_CWMODE(int mode){
	char answer[100] = {0};
	char command[20] = {0};
	sprintf(command, "AT+CWMODE=%d\r\n", mode);

	if (WIFI_Exec_Cmd_Get_Answer(command, answer) != 0 ) {
		return 1;
	}

	if ( strncmp(answer, "busy", 4) == 0 ){
		Log_Message("busy");
		return 1;
	}

	if(strncmp(answer, "OK", 2) != 0){
		WIFI_Read_Line(answer,100, 200000);
		return 1;
	}

	return 0;
}

int WIFI_Set_CIPMUX(int mode){
	char answer[100] = {0};
	char command[20]= {0};
	sprintf(command, "AT+CIPMUX=%d\r\n", mode);

	is_new_line=0;
	WIFI_Send_Command(command, 0);
	if ( WIFI_Read_Line(answer,100, 400000) != 0 ) {
		return 1;
	}

#ifndef ESP_DISABLE_ECHO
	if (WIFI_Read_Line(answer,100, 400000) != 0) {
		return 1;
	}
#endif

	if (strncmp("CIPSERVER must be 0", answer, 19) == 0){
		Log_Message("CIPSERVER must be 0");
		return 1;
	}

	if (strncmp("Connection exists", answer, 17) == 0){
		Log_Message("Connection exists"); // TODO retry?
		return 1;
	}

	if (WIFI_Read_Line(answer,100, 400000) != 0 ) {
		return 1;
	}

	return strncmp(answer, "OK", 2);
}

int WIFI_Get_Status(char* ip, char* mac){
	char answer[512] ={0};
	int i;

	WIFI_Send_Command("AT+CIFSR\r\n", 0);
	if (WIFI_Read_Line(answer, 100, 800000) != 0 ){
		return 1;
	}
	// TODO CHECK ESP_DISABLE_ECHO

	//+CIFSR:APIP,\"192.168.4.1\"\r\n\r\r\n\r\n2 \r\node:(3,7)\r\n
	if (WIFI_Read_Line(answer,100, 8000000) != 0){
		return 1;
	}

	// IP is not acquired yet
	if(strncmp("AT+CIFSR\r\r", answer, 10) == 0){
		strcpy(ip, "not acquired");
		return 1;
	}

	if( strncmp("+CIFSR:", answer, 7) != 0){
		return 1;
	}

	//APIP/STAIP
	int starts_from = 0;
	if ( strncmp("+CIFSR:APIP", answer, 11) == 0 ){
		starts_from = 13;
	}else{
		starts_from = 14;
	}

	int ip_remaining=20;

	for(i=0;i<ip_remaining; i++){
		if( answer[starts_from+i] == '"' ){
			ip_remaining=0;
			ip[starts_from+i] = '\0';
		}else{
			ip[i] = answer[starts_from+i];
		}
	}
	ip[i+1] = '\0';

	//+CIFSR:APMAC,\"1a:fe:34:f3:3d:0e\"\r\n\r\node:(3,7)\r\n
	if (WIFI_Read_Line(answer,100, 8000000) != 0){
		return 1;
	}
	//+CIFSR:STAMAC/APMAC
	if( strncmp("+CIFSR:", answer, 7) != 0){
		return 1;
	}

	if ( strncmp("+CIFSR:APMAC", answer, 12) == 0 ){
		starts_from = 14;
	}else{
		starts_from = 15;
	}

	int mac_remaining=20;
	for(i=0; i<mac_remaining; i++){
		if( answer[starts_from+i] == '"' ){
			mac_remaining=0;
			mac[starts_from+i] = '\0';
		}else{
			mac[i] = answer[starts_from+i];
		}
	}
	mac[i+1] = '\0';

	if (WIFI_Read_Line(answer,100, 8000000) != 0){ // \r\n
		return 1;
	}

	if (WIFI_Read_Line(answer,100, 8000000) != 0){
		return 1;
	}

	return strncmp(answer, "OK", 2);
}

int WIFI_Connect(char* wifi_name, char* wifi_pass){
	char answer[100];

	if (WIFI_Set_CWMODE(1) != 0) {
		return 1;
	}

	char command[200] = {0};
	sprintf(command, "AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_name, wifi_pass);

	WIFI_Exec_Cmd_Get_Answer(command, answer);
	if (strncmp(answer, "OK", 2) != 0) {
		WIFI_Read_Line(answer,100, 200000);
		if (strncmp(answer, "OK", 2) != 0){
			return 1;
		}
	}

	return 0;
}

int WIFI_Disconnect(){
	char answer[100];
	WIFI_Exec_Cmd_Get_Answer("AT+CWQAP\r\n", answer);
	if (strncmp(answer, "OK", 2) != 0) {
		return 1;
	}

	return 0;

}

int WIFI_Set_CWSAP(char* ssid, char* password, uint8_t channel, uint8_t ecn){
	char answer[100];
	char command[200] = {0};
	sprintf(command, "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n", ssid, password, channel, ecn);

	if (WIFI_Exec_Cmd_Get_Answer(command, answer) != 0) {
		return 1;
	}

	return strncmp(answer, "OK", 2);
}

/*
 * +CWLAP:(3,\"kiryam\",-64,\"d0:17:c2:64:22:d4\",11)\r\n
 */
int WIFI_Parse_Point_Answer(char* answer, WIFI_Point *point) {
	if( strncmp("+CWLAP", answer, 6) == 0) {
		const char delim[2] =",";
		char *token;
		int tokenNum = 0;
		token = strtok(answer, delim);
		while( token != NULL ) {
			if( tokenNum++ == 1){
				strncpy(point->name, &token[1], strlen(token)-2);
				point->name[strlen(token)-2] = '\0';
				return 0;
			}
			token = strtok(NULL, delim);
		}

	}
	return 1;
}

int WIFI_Retreive_List(){
	//if (WIFI_Disconnect() != 0) {
	//	return 1;
	//}

	if (WIFI_Set_CWMODE(1) != 0) {
		return 1;
	}

	is_new_line = 0;
	WIFI_Send_Command("AT+CWLAP\r\n", 0);

	char answer[500] = {0};
	if ( WIFI_Read_Line(answer, 100, 8000000) == 1 ){
		return 1;
	}

#ifndef ESP_DISABLE_ECHO
	if( strcmp(answer, "AT+CWLAP\r\r\n") == 1 ){
		return 1;
	}
#endif

	Log_Message("AT+CWLAP ok");
	return 0;
}

// TODO
int WIFI_Resolve_DNS(char* host, char* ipaddr){
	return 0;
}

int WIFI_TCP_Connect(char* host, int port) {
	char answer[100] = {0};
	char port_str[6] = {0};
	itoa(port, port_str, 10);
	Log_Message("TCP connect");
	Log_Message(host);
	Log_Message(port_str);

	if ( WIFI_Set_CIPMUX(1) ) {
		return 1;
	}

	// TODO resolve DNS
	// AT+CIPSTART=0,\"TCP\",\"192.168.1.216\",7000\r\n
	char command[100] ="AT+CIPSTART=0,\"TCP\",\"";
	strcat(command, host);
	strcat(command, "\",");
	strcat(command, port_str);
	strcat(command, "\r\n");

	WIFI_Exec_Cmd_Get_Answer(command, answer); // TODO
	WIFI_Read_Line(answer, 100, 200000);
	if (strncmp(answer, "OK", 2) != 0) {
		return 1;
	}

	return 0;
}

int WIFI_TCP_Send(uint8_t conn_id, uint8_t* data, unsigned int bytes_count){ // TODO MAKE MUTEX
	if(conn_id != 0){
		Log_Message("conn_id != 0");
	}
	char answer[512] ={0};
	char command[20] = {0};
	sprintf(command, "AT+CIPSEND=%d,%d\r\n",conn_id, bytes_count);


	is_new_line = 0;
	WIFI_Send_Command(command,0);

	WIFI_Read_Line(answer, 100, 800000);
	WIFI_Read_Line(answer, 100, 800000);

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 100, 800000);
#endif

	is_welcome_byte=0;

	if (strncmp(answer, "\r\n", 2) != 0) {
		WIFI_Read_Line(answer, 100, 800000);

		if (strncmp(answer, "OK", 2) != 0) {
			Log_Message("Not OK");
			//return 1;
		}
	}else if (strncmp(answer, "OK", 2) != 0) {
		Log_Message("Not OK");
		//return 1;
	}else if (strncmp(answer, "link is not valid", 17) == 0) {
		Log_Message("Link is not valid");
		return 1;
	}

	if( wait_welcome_byte(800000) != 0 ){
		Log_Message("No welcome message");
		//return 1;
	}
	is_new_line = 0;
	WIFI_Send_Bytes(data, bytes_count, 0);

	is_new_line = 0;
	WIFI_Send_Command("\r\n", 0);

	WIFI_Read_Line(answer, 512, 800000);  // /r/n

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 512, 800000);
#endif

	if( strncmp(answer, " busy s...\r\n", 12) == 0 ){
		int remained=100;
		while( (remained--) > 0  ){
			if (WIFI_Read_Line(answer, 512, 800000) != 0){
				Log_Message("err");
				continue;
			}

			if (strncmp(answer, "SEND OK", 7) == 0){
				return 0;
			 }

			if( strncmp(answer, "\r\n", 2) == 0){
				continue;
			}
			if( strncmp(answer, "OK", 2) == 0){
				Log_Message("OK");
			}else{
				Log_Message("OK");
			}
		}
		return 1;
	}
	WIFI_Read_Line(answer, 512, 3200000);

	if (strncmp(answer, "SEND OK", 6) != 0) {
		Log_Message("Send not ok");
		//return 1;
	}

	return 0;
}

int WIFI_TCP_Disconnect(uint8_t conn_id){
	char answer[100] = {0};
	char command[20] = "";
	sprintf(command, "AT+CIPCLOSE=%d\r\n", conn_id);

	WIFI_Send_Command(command, 0);

	if (WIFI_Read_Line(answer, 100, 400000)  ){
		return 1;
	}

#ifndef  ESP_DISABLE_ECHO
	if (WIFI_Read_Line(answer, 100, 400000)  ){
		return 1;
	}
#endif

	if( strncmp(answer, "link is not", 11) == 0 ){
		return 2;
	}

	char wait_answer[15];
	sprintf(wait_answer, "%d,CLOSED\r\n", conn_id);
	if (strcmp(answer, wait_answer) != 0) {

		return 1;
	}

	return 0;
}

int WIFI_Set_CIPSERVER(int mode, int port){
	char answer[100];
	char command[30] = {0};
	sprintf(command, "AT+CIPSERVER=%d", mode);

	if( mode == 1 ){
		strcat(command, ",");
		itoa(port, &command[strlen(command)], 10);
	}
	strcat(command, "\r\n");

	WIFI_Send_Command(command, 0);
	WIFI_Read_Line(answer, 100, 200000);

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 100, 200000);
#endif

	if (mode ==0 ){
		if (WIFI_Read_Line(answer, 100, 200000) ) {
			return 1;
		}
		if (strncmp(answer, "ERROR", 5) == 0) {
			return 0; // already closed // TODO
		}
		if (strncmp(answer, "OK", 2) == 0) {
			return 0;
		}
		if ( WIFI_Read_Line(answer, 100, 200000) ){
			return 1;
		}

		if (strncmp(answer, "OK", 2) != 0) {
			return 1;
		}
	}else{
		if (strncmp(answer, "no change", 9) == 0) {
			WIFI_Read_Line(answer, 100, 200000); // \r\n
		}

		WIFI_Read_Line(answer, 100, 200000);
		if (strncmp(answer, "OK", 2) != 0) {
			return 1;
		}
	}

	return 0;

}

int WIFI_Server_Start(int port){
	//if( WIFI_Set_CWMODE(1) ) {
	//	Log_Message("CWMODE=1 ERROR");
	//	return 1;
	//}

	if ( WIFI_Set_CIPSERVER(0, 0) ){
		Log_Message("CIPSERVER=0 ERROR");
		return 1;
	}
	Log_Message("CIPSERVER=0 OK");

	if ( WIFI_Set_CIPMUX(0) ) {
		Log_Message("CIPMUX=0 ERROR");
		return 1;
	}
	Log_Message("CIPMUX=0 OK");

	if ( WIFI_Set_CIPMODE(0) ) {
		Log_Message("CIPMODE=0 ERROR");
		return 1;
	}
	Log_Message("CIPMODE=0 OK");

	if ( WIFI_Set_CIPMUX(1) ) {
		Log_Message("CPIMUX=1 ERROR");
		return 1;
	}
	Log_Message("CPIMUX=1 OK");

	if (WIFI_Set_CIPSERVER(1, port) ) {
		Log_Message("CIPSERVER=1 ERROR");
		return 1;
	}
	Log_Message("CIPSERVER=1 OK");

	return 0;
}

int WIFI_ATE(uint8_t mode){
	char command[10]={0};
	char answer[100] = {0};
	sprintf(command, "ATE%d\r\n", mode);

	if (WIFI_Exec_Cmd_Get_Answer(command, answer) != 0) {
		return 1;
	}

	if (strncmp(answer, "OK", 2) != 0){
		return 1;
	}

	if( mode ==0 ) {
		Log_Message("Echo disabled");
	}else{
		Log_Message("Echo enabled");
	}

	return 0;
}

void WIFI_Init(){
	MY_USART_Init();
	//sleepMs(4000);
	//if( WIFI_Reset() != 0 ){
	//	Log_Message("Reset failed");
	//}
	//wait_new_line(800000);
	sleepMs(20000);

	if (WIFI_ATE(0) != 0) {
		Log_Message("Failed to disable echo");
		// FIXME ?? RESET
	}

	if ( WIFI_Test() == 0) {
		Log_Message("Wifi test ok");
		if(WIFI_Set_CWMODE(1) == 0){
			Log_Message("WIFI_Set_CWMODE ok");
			wifi_init_ok = 1;
		}
	}else{
		Log_Message("Wifi test failed");
	}
}
