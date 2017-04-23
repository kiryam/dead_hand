#include <stdio.h>
#include "message_queue.h"
#include "common.h"
#include "ipd_parser.h"
#include <string.h>

static message_command* message_queue_front = NULL;
static message_command* message_queue_rear = NULL;
static int message_queue_count = 0;


static ipd_parser* message_data_queue[MAX_PENDING_CONNETION*2] = {0};
static unsigned int message_data_cursors[MAX_PENDING_CONNETION] = {0};

int message_data_queue_add(ipd_parser* parser){
	int conn_id = parser->conn_id;
	if ( conn_id >= MAX_PENDING_CONNETION ) {
		Log_Message("Maximum pending connections");
		return 1;
	}
	if (message_data_cursors[conn_id] >= MAX_PENDING_DATA){
		Log_Message("Data message queue overloaded");
		return 1;
	}

	parser->next = NULL;

	if( message_data_queue[conn_id<<1] == NULL) {
		message_data_queue[conn_id<<1] = message_data_queue[(conn_id<<1)+1] = parser;
	} else {
		message_data_queue[(conn_id<<1)+1]->next = parser;
		message_data_queue[(conn_id<<1)+1] = parser;
	}

	message_data_cursors[conn_id]++;

	return 0;
}

ipd_parser* message_data_queue_get_by_conn_id(unsigned int conn_id){
	if ( conn_id >= MAX_PENDING_CONNETION ) {
		Log_Message("Conn_id to big");
		return NULL;
	}

	if (message_data_cursors[conn_id] == 0){
		return NULL;
	}

	message_data_cursors[conn_id]--;
	ipd_parser* msg = message_data_queue[conn_id<<1]; // get from front
	message_data_queue[conn_id<<1] = msg->next; // front set to next

	//if (message_data_cursors[conn_id] == 0) {
	//	message_data_queue[(conn_id<<1)+1] = NULL;
	//}

	return msg;
}

ipd_parser*  message_data_queue_get(){
	ipd_parser* ret = NULL;
	for(int i=0;i<MAX_PENDING_CONNETION; i++){
		ret = message_data_queue_get_by_conn_id(i);
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
