#include <stdio.h>
#include "message_queue.h"
#include "common.h"

static message_command* message_queue_front = NULL;
static message_command* message_queue_rear = NULL;
static int message_queue_count = 0;

static message_data* message_data_queue_front = NULL;
static message_data* message_data_queue_rear = NULL;

static int message_data_queue_count = 0;

message_data* message_data_queue_add(int conn_id, char* buff){
	if(message_data_queue_count+1 > MAX_PENDING_MESSAGES_DATA){
		Log_Message("Data message queue overloaded");
		return NULL; // TODO QUEUE overloaded notify
	}

	message_data_queue_count++;
	message_data* msg = (message_data*)malloc_c(sizeof(message_data));
	strncpy(msg->line, buff, MESSAGE_DATA_MAX_SIZE);
	msg->target = conn_id;
	msg->next = NULL;

	if( message_data_queue_front == NULL ){
		message_data_queue_front = message_data_queue_rear = msg;
	}else{
		message_data_queue_rear->next = (message_data*)msg;
		message_data_queue_rear = msg;
	}

	return msg;
}

message_data* message_data_queue_get(){
	if( message_data_queue_front == NULL ){
		return NULL;
	}

	message_data_queue_count--;
	message_data* msg = message_data_queue_front;
	message_data_queue_front = (message_data*)msg->next;

	return msg;
}

message_command* message_queue_add(char* buff){
	if(message_queue_count+1 > MAX_PENDING_MESSAGES){
		Log_Message("Message queue overloaded");
		return NULL; // TODO QUEUE overloaded notify
	}

	message_queue_count++;
	message_command* msg = (message_command*)malloc_c(sizeof(message_command));
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
