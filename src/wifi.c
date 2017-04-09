#include <stdio.h>
#include "wifi.h"
#include "log.h"
#include "message_queue.h"
#include "server.h"


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

#define PROTOCOL_LOG_MAX_LENGTH 1024*1

#ifdef PROTO_LOG
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
#endif

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
			if( msg->line[0] != '\0' ){
				for(int i=0; i<MAX_NEWLINE_CALLBACK_COUNT; i++){
					if (new_line_handlers[i] != NULL){
						new_line_handlers[i](&msg->line);
					}
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
				Log_Message("Message too long");
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

		recv_message_buff[recv_message_data_bytes_read++] = cur_byte;

		if ( recv_message_data_bytes_read == recv_message_size) {
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

#ifdef PROTO_LOG
		protocol_log_byte(wifi_buff[wifi_buff_pos], DIR_IN);
#endif

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
				recv_message_size_tmp[0] = '\0';
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
	base_timer.TIM_Period = 1;
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
#ifdef PROTO_LOG
		protocol_log_byte(command[i], DIR_OUT);
#endif
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
	}
}

void WIFI_Send_Bytes(uint8_t* bytes, unsigned int bytes_count,  uint8_t timeout){
	// TODO implement timeout
	for (unsigned int i=0; i<=bytes_count; i++){
		USART_SendData(USART1, bytes[i]);
#ifdef PROTO_LOG
		protocol_log_byte((char)bytes[i], DIR_OUT);
#endif
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

	return strncmp("OK", answer,2);
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


	if(strncmp(answer, "\r\n", 2) == 0){
		WIFI_Read_Line(answer,100, 200000);
		Log_Message(answer);
		//return 1;
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

int read_to_quote(char* answer, char* out){
	int i;
	int ip_remaining=20;

	for(i=0;i<ip_remaining; i++){
		if( answer[i] == '"' ){
			ip_remaining=0;
			out[i] = '\0';
		}else{
			out[i] = answer[i];
		}
	}
	out[i+1] = '\0';

	return 0;
}

int WIFI_Get_Status(char* ip, char* sta_ip, char* mac, char* sta_mac){
	char answer[512] ={0};
	ip[0] = '\0';
	mac[0] = '\0';
	is_new_line = 0;
	int lines_estimated = 6;

	WIFI_Send_Command("AT+CIFSR\r\n", 0);

	while((lines_estimated--) > 0){
		if (WIFI_Read_Line(answer, 500, 800000) != 0 ){
			return 1;
		}
		Log_Message(answer);
		if( strncmp("+CIFSR:", answer, 7) == 0){
			char tmp[IP_MAX_LEN] = {0};
			if( strncmp(&answer[7], "APIP", 4) == 0 ){
				read_to_quote(&answer[13], tmp);
				strncpy(ip, tmp, IP_MAX_LEN);
			}else if( strncmp(&answer[7], "STAIP", 5) == 0){
				read_to_quote(&answer[14], tmp);
				strncpy(sta_ip, tmp, IP_MAX_LEN);
			}else if(  strncmp(&answer[7], "APMAC", 5) == 0 ){
				read_to_quote(&answer[14], tmp);
				strncpy(mac, tmp, MAC_MAX_LEN);
			}else if( strncmp(&answer[7], "STAMAC", 6) == 0 ){
				read_to_quote(&answer[15], tmp);
				strncpy(sta_mac, tmp, MAC_MAX_LEN);
			}
		}else{
			break;
		}
	}

	return strncmp(answer, "OK", 2);
}

int WIFI_Connect(char* wifi_name, char* wifi_pass){
	char answer[100]={0};
	char command[100] = {0};

	if (WIFI_Set_CWMODE(1) != 0) {
		Log_Message("Set CWMODE 1 Error");
		return 1;
	}

	sprintf(command, "AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_name, wifi_pass);

	if (WIFI_Exec_Cmd_Get_Answer("AT+CWJAP?\r\n", answer)  != 0){
		return 1;
	}

	if (WIFI_Read_Line(answer, 100, 800000) != 0 ){
		return 1;
	}

	if (strncmp(answer, "OK", 2) != 0){
		return 1;
	}

	//if (
	WIFI_Exec_Cmd_Get_Answer(command, answer);
			//!= 0)
	//{
		//return 1;
	//}
	/*if ( answer[0]=='O' ) {
		WIFI_Read_Line(answer,100, 200000);
		if (strncmp(answer, "OK", 2) != 0){
			return 1;
		}
	}*/

	return 0;
}

int WIFI_Disconnect(){
	char answer[100]={0};
	WIFI_Exec_Cmd_Get_Answer("AT+CWQAP\r\n", answer);
	return strncmp(answer, "OK", 2);
}

// 0-82
int WIFI_Power(unsigned int power){
	char answer[100]={0};
	char command[20] = {0};
	sprintf(command, "AT+RFPOWER=%d\r\n", power);
	WIFI_Exec_Cmd_Get_Answer(command, answer);
	return strncmp(answer, "OK", 2);
}

int WIFI_Restore(){
	char answer[100]={0};
	WIFI_Exec_Cmd_Get_Answer("AT+RESTORE\r\n", answer);
	return strncmp(answer, "OK", 2);
}

int WIFI_Set_CWSAP(char* ssid, char* password, uint8_t channel, uint8_t ecn){
	char answer[100] = {0};
	char command[200] = {0};
	sprintf(command, "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n", ssid, password, channel, ecn);

	if (WIFI_Exec_Cmd_Get_Answer(command, answer) != 0) {
		return 1;
	}

	if(strncmp(answer, "\r\n", 2) == 0){
		WIFI_Read_Line(answer,100, 200000);
		Log_Message(answer);
		//return 1;
	}

	return strncmp(answer, "OK", 2);
}

/*
 * +CWLAP:(3,\"kiryam\",-64,\"d0:17:c2:64:22:d4\",11)\r\n
 */
int WIFI_Parse_Point_Answer(char* answer, WIFI_Point *point) {
	if( answer[0] == '\r' && answer[1] == 'C' ){
		answer[0] = '+';
	}
	if( strncmp("+CWLAP", answer, 6) == 0) {
		char tmp[200] = {0};
		strncpy(tmp, answer, 200);
		const char delim[2] =",";
		char *token;
		int tokenNum = 0;
		token = strtok(tmp, delim);
		while( token != NULL ) {
			if( tokenNum++ == 1){
				char point_name[WIFI_POINT_MAX_LEN] ={0};
				strncpy(point_name, &token[1], strlen(token)-2);
				strncpy(point->name, point_name, WIFI_POINT_MAX_LEN);
				return 0;
			}
			token = strtok(NULL, delim);
		}

	}
	return 1;
}

int WIFI_Retreive_List(WIFI_List_Result* result){
	result->found =0;

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

	int readed_count=MAX_POINTS_COUNT;
	while( (readed_count--) > 0){
		if ( WIFI_Read_Line(answer, 100, 8000000) == 1 ){
			Log_Message("Points retrieve loop went too long");
			return 1;
		}

		if( strncmp(answer, "OK", 2) == 0 ) {
			return 0;
		} else if(strncmp(answer, "\r\n", 2) == 0) {
			continue;
		} else {
			WIFI_Point* point = malloc_c(sizeof(WIFI_Point));

			if ( WIFI_Parse_Point_Answer(answer, point) == 0 ){
				Log_Message("Parsed ok");
				Log_Message(point->name);
				result->points[result->found++] = point;
			}else{
				free_c(point);
				Log_Message("Failed to parse");
				Log_Message(answer);
			}
		}
	}

	Log_Message("AT+CWLAP fail");
	return 1;
}

void WIFI_List_Result_Free(WIFI_List_Result* result){
	for(int i=0; i< result->found; i++){
		free_c(result->points[i]);
	}
	free_c(result);
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

int WIFI_TCP_Send(uint8_t conn_id, uint8_t* data, unsigned int bytes_count){
	if( bytes_count > MAX_PACKET_LEN ){
		Log_Message("Max packet len");
		return 1;
	}

	if(open_connections[conn_id] == 0){
		Log_Message("Connection already closed");
		return 1;
	}
	char answer[100] = {0};
	char command[20] = {0};
	sprintf(command, "AT+CIPSEND=%d,%d\r\n", conn_id, bytes_count);

	int retry_remained = 3;
	is_welcome_byte = 0;
	is_new_line = 0;

	WIFI_Send_Command(command,0);

	WIFI_Read_Line(answer, 100, 200000);
	WIFI_Read_Line(answer, 100, 200000);

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 100, 800000);
#endif

	if (strncmp(answer, "\r\n", 2) != 0) {
		WIFI_Read_Line(answer, 100, 400000);

		if (strncmp(answer, "OK", 2) != 0) {
			Log_Message("Not OK");
			//return 1;
		}
	}

	if( wait_welcome_byte(200000) != 0 ){
		Log_Message("No welcome message");
		//return 1;
	}

	WIFI_Send_Bytes(data, bytes_count, 0);
	WIFI_Send_Command("\r\n", 0);

	WIFI_Read_Line(answer, 100, 800000);  // /r/n

#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 100, 800000);
#endif

	if( strncmp(&answer[1], "\r\n", 2) == 0 ){
		WIFI_Read_Line(answer, 100, 800000);

		if( strncmp(answer, "busy", 4) == 0 ){
			while( (retry_remained--) > 0  ){
				if (WIFI_Read_Line(answer, 100, 800000) != 0){
					continue;
				}

				if( strncmp(answer, "Recv", 4) == 0 ){
					return 0;
				}

				if (strncmp(answer, "\r\n", 2) == 0){
					if (WIFI_Read_Line(answer, 100, 800000) == 0){
						return 0;
					}
					//Recv 93 bytes\r\n

				}

				if (strncmp(answer, "SEND OK", 7) == 0){
					return 0;
				}
			}
			return 1;
		}
	}

	if (WIFI_Read_Line(answer, 100, 800000)){
		return 1;
	}
	//Recv 93 bytes\r\n

	if (WIFI_Read_Line(answer, 100, 800000)){
		return 1;
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

//#ifndef ESP_DISABLE_ECHO
	WIFI_Read_Line(answer, 100, 200000); // WIFI CONNECTED
//#endif

	if (mode ==0 ){
		if (strncmp(answer, "ERROR", 5) == 0) {
			return 0; // already closed // TODO
		}
		if (strncmp(answer, "OK", 2) == 0) {
			return 0;
		}

		if( strncmp(answer, "\r\n", 2) == 0 ){
			WIFI_Read_Line(answer, 100, 200000);
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

		//WIFI_Read_Line(answer, 100, 200000);
		if (strncmp(answer, "OK", 2) != 0) {
			return 1;
		}
	}

	return 0;

}


int WIFI_ATE(uint8_t mode){
	char command[10]={0};
	char answer[100] = {0};
	sprintf(command, "ATE%d\r\n", mode);

	if (WIFI_Exec_Cmd_Get_Answer(command, answer) != 0) {
		return 1;
	}

	if( strncmp(answer, "\r\n", 2) == 0){
		if (WIFI_Read_Line(answer, 100, 800000) != 0 ){
			return 1;
		}
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
	sleepMs(2000);

	if( WIFI_Reset() != 0 ){
		Log_Message("Reset failed");
	}
	sleepMs(2000);

	if (WIFI_ATE(0) != 0) {
		Log_Message("Failed to disable echo");
		// FIXME ?? RESET
	}

	if ( WIFI_Test() == 0) {
		Log_Message("Wifi test ok");
		if(WIFI_Set_CWMODE(3) == 0){
			Log_Message("WIFI_Set_CWMODE ok");
			wifi_init_ok = 1;
		}

		if(WIFI_Power(82) == 0){
			Log_Message("Set RX TX power ok");
		}
	}else{
		Log_Message("Wifi test failed");
	}
}
