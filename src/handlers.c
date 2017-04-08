#include <stdio.h>
#include "request.h"


void handler_index(Request* request,  char* response) {
	Log_Message("index");

	//char *page = "<html><head><title>Dead hand configure</title></head><body><h1>Set connection</h1><form action=\"/set_passwd\">Password: <input name=\"password\" /><br /><input type=\"submit\" value=\"Connect\" /></form></body></html>";
	page_index(response, "<h2 class=\"sub-header\">Section title</h2><div class=\"table-responsive\">Hello</div>");
	//strncpy(response, page_index(), RESPONSE_MAX_LEN);
}

void handler_set_password(Request* request, char* response){
	if( request->type == POST ) {
		Log_Message("set passwd");
		for(int i=0; i< request->data_count;i++){
			Request_Data* data= request->data[i];
			if( strncmp(data->key, "password", 8) == 0 && data->key[8] == '\0' ){
				config_set("password", data->value);
			}
		}

		char answer[512]={0};
		sprintf(answer, "<h2 class=\"sub-header\">Password set</h2><div class=\"table-responsive\">Password set to <code>%s</code></div>", config_get("password"));
		page_index(response, answer);
	} else {
		page_index(response, "<h2 class=\"sub-header\">Set connect password</h2><div class=\"table-responsive\">"
				"<form method=\"POST\">Password: <input name=\"password\" /><br /><input name=\"password2\" /><input type=\"submit\" value=\"Connect\" /></form>"
				"</div>");
	}
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

void handler_favicon(Request* request, char* response){
	Log_Message("Favicon");
	strcpy(response, "Favicon");
	// TODO
}

void handler_404(Request* request, char* response){
	Log_Message("404");
	strcpy(response, "404 Page not found");
}

void handler_metrics(Request* request, char* response){
	Log_Message("metric");
	sprintf(response, "memory_used %d\nmemory_max_used %d\n", get_memory_allocated_total(), get_memory_max());
}
