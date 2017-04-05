#include <stdio.h>
#include "request.h"


void handler_index(Request* request,  char* response) {
	Log_Message("index");

	char *page = "<html><head><title>Dead hand configure</title></head><body><h1>Set connection</h1><form action=\"/set_passwd\">Password: <input name=\"password\" /><br /><input type=\"submit\" value=\"Connect\" /></form></body></html>";
	strcpy(response, page);
}

void handler_set_password(Request* request, char* response){
	Log_Message("set passwd");
	const char s[2] = "&";
	char *token;
	token = strtok(&request->path[12], s);
	while( token != NULL ) {
		if (strncmp("password", token, 8) == 0){
			config_set("password", &token[9]);
		}
		token = strtok(NULL, s);
	}

	sprintf(response, "Password set to %s", config_get("password"));
}

#ifdef PROTO_LOG
void handler_get_protocol_log(Request* request, char* response){
	Log_Message("Get protocol log");
	strncpy(response, get_protocol_log(), RESPONSE_MAX_LEN);
}
#endif


void handler_memory(Request* request, char* response){
	Log_Message("Memory");
	sprintf(response, "Allocated memory: %d of %d", get_memory_allocated_total(), get_memory_max());
}


void handler_metrics(Request* request, char* response){
	Log_Message("metric");
	sprintf(response, "memory_used %d\nmemory_max_used %d\n", get_memory_allocated_total(), get_memory_max());
}
