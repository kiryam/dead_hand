#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


#define MESSAGE_COMMAND_SIZE 256
#define MAX_PENDING_CONNETION 5
#define MAX_PENDING_DATA 10
#define MAX_DATA_IN_QUEUE MAX_PENDING_CONNETION*MAX_PENDING_DATA

typedef struct {
	unsigned int conn_id;
	char* message;
	size_t message_length;
} message_data;

void ipd_queue_add(message_data* packet);

message_data* ipd_queue_get();
message_data* ipd_queue_get_by_conn_id(unsigned int conn_id);

#ifdef __cplusplus
}
#endif

#endif
