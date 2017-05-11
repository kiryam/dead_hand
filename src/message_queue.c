#include <stdio.h>
#include "message_queue.h"
#include "common.h"
#include "ipd_parser.h"
#include <string.h>
#include "log.h"

static message_data* message_data_queue[MAX_PENDING_CONNETION*MAX_PENDING_DATA] = {0};
static unsigned int message_data_cursors[MAX_PENDING_CONNETION*2] = {0};

// TODO CONN_ID
// message_data_cursors[conn_id << 0] // read_cursor
// message_data_cursors[conn_id << 1] // write_cursor
void ipd_queue_add(message_data* packet){
	unsigned int* write_cursor = &message_data_cursors[(packet->conn_id<<1)+1];

	if (*write_cursor >= MAX_PENDING_DATA) {
		*write_cursor = 0;
	}

	if (message_data_queue[*write_cursor] != NULL) {
		Log_Message("Data buffer overflow");
		free_c(message_data_queue[*write_cursor]);
		message_data_queue[*write_cursor] = NULL;
	}

	message_data_queue[*write_cursor] = packet;
	*write_cursor=*write_cursor+1;
}

message_data* ipd_queue_get_by_conn_id(unsigned int conn_id) {
	unsigned int* read_cursor = &message_data_cursors[(conn_id<<1)+0];

	if (*read_cursor >= MAX_PENDING_DATA){
		*read_cursor = 0;
	}

	message_data *packet = message_data_queue[*read_cursor];
	if (packet != NULL){
		message_data_queue[*read_cursor] = NULL;
		*read_cursor = *read_cursor+1;
	}

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
