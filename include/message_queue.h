#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


#define MESSAGE_COMMAND_SIZE 128
#define MESSAGE_DATA_MAX_SIZE 1024*4

#define MAX_PENDING_MESSAGES 200
#define MAX_PENDING_MESSAGES_DATA 3

typedef struct {
	char line[MESSAGE_COMMAND_SIZE];
	struct message_command* next;
} message_command;

typedef struct {
	char line[MESSAGE_DATA_MAX_SIZE];
	int  target;
	struct message_data* next;

} message_data;


#ifdef __cplusplus
}
#endif

#endif
