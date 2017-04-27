#include <stdio.h>
#include "message_queue.h"
#include "common.h"
#include "ipd_parser.h"
#include <string.h>

static message_command* message_queue_front = NULL;
static message_command* message_queue_rear = NULL;
static int message_queue_count = 0;


static message_data* message_data_queue[MAX_PENDING_CONNETION*MAX_PENDING_DATA] = {0};
static unsigned int message_data_cursors[MAX_PENDING_CONNETION*2] = {0};

#define IPD_RAW_QUEUE_SIZE 4
static char* ipd_raw_queue[IPD_RAW_QUEUE_SIZE] = {0};
static  int ipd_raw_write_cursor = 0;
static int ipd_raw_read_cursor = 0;

// TODO CONN_ID
// message_data_cursors[conn_id << 0] // read_cursor
// message_data_cursors[conn_id << 1] // write_cursor
void ipd_queue_add(unsigned int conn_id, char* buff, size_t length){
	unsigned int* write_cursor = &message_data_cursors[(conn_id<<1)+1];
	if (*write_cursor >= IPD_RAW_QUEUE_SIZE) {
		*write_cursor = 0;
	}

	if (message_data_queue[*write_cursor] != NULL) {
		Log_Message("Data buffer overflow");
		free_c(message_data_queue[*write_cursor]);
		message_data_queue[*write_cursor] = NULL;
	}

	message_data* packet=(message_data*)malloc_c(sizeof(message_data));
	char* msg = (char*)malloc_c(sizeof(char)*length);
	if (msg == NULL){
		free_c(packet);
		return;
	}
	strncpy(msg, buff, length);

	packet->conn_id = conn_id;
	packet->message_length = length;
	packet->message = msg;

	message_data_queue[*write_cursor] = packet;
	*write_cursor=*write_cursor+1;
}

message_data* ipd_queue_get_by_conn_id(unsigned int conn_id) {
	unsigned int* read_cursor = &message_data_cursors[(conn_id<<1)+0];

	if (*read_cursor >= IPD_RAW_QUEUE_SIZE){
		*read_cursor = 0;
	}

	message_data *packet = message_data_queue[*read_cursor];
	if (packet != NULL){
		message_data_queue[*read_cursor] = NULL;
	}

	*read_cursor = *read_cursor+1;
	return packet;
}

message_data*  ipd_queue_get(){
	message_data* ret = NULL;
	for(int i=0;i<MAX_PENDING_CONNETION; i++){
		ret = ipd_queue_get_by_conn_id(i);
		if (ret != NULL){
			return ret;
		}
	}

	return NULL;
}

message_command* message_queue_add(char* buff){
	if(message_queue_count+1 > MAX_PENDING_MESSAGES){
		Log_Message("Message queue overloaded");
		return NULL; // TODO QUEUE overloaded notify
	}

	message_queue_count++;
	message_command* msg = (message_command*)malloc_c(sizeof(message_command));
	if (msg == NULL){
		Log_Message("Out of memory");
		return NULL;
	}
	//memset(msg, 0, sizeof(message_command));
	strncpy(msg->line, buff, MESSAGE_COMMAND_SIZE);
	msg->next = NULL;

	if( message_queue_front == NULL){
		message_queue_front = message_queue_rear = msg;
	} else {
		message_queue_rear->next = msg;
		message_queue_rear = msg;
	}

	return msg;
}

message_command* message_queue_get(){
	if( message_queue_front == NULL ){
		return NULL;
	}

	message_queue_count--;
	message_command* msg = message_queue_front;
	message_queue_front = msg->next;

	return msg;
}
