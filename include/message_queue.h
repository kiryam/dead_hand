#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipd_parser.h"

#define MESSAGE_COMMAND_SIZE 128

#define MAX_PENDING_MESSAGES 200


#define MAX_PENDING_CONNETION 5
#define MAX_PENDING_DATA 3
#define MAX_DATA_IN_QUEUE MAX_PENDING_CONNETION*MAX_PENDING_DATA

typedef struct {
	char line[MESSAGE_COMMAND_SIZE];
	struct message_command* next;
} message_command;


int message_data_queue_add(ipd_parser* parser);
ipd_parser*  message_data_queue_get();

#ifdef __cplusplus
}
#endif

#endif
