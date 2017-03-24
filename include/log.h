#ifndef LOG_H
#define LOG_H


#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f2xx.h"
#include "common.h"

#define MAX_LOG_ENTRIES 20
#define MAX_MESSAGE_LENGTH 30
typedef struct {
	char message[MAX_MESSAGE_LENGTH];
	int count;
} Log_Entry;

void Log_Message(char* str);
void Log_Get_Messages(Log_Entry* target_list, int limit);

void Log_Init();
void controller_log(int btn);
void render_log();

#ifdef __cplusplus
}
#endif

#endif
