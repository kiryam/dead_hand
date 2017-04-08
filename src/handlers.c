#include <stdio.h>
#include <string.h>
#include "request.h"


void handler_index(Request* request,  char* response) {
	Log_Message("index");
	page_index(response, "<h2 class=\"sub-header\">Section title</h2><div class=\"table-responsive\">Hello</div>");
}

void handler_set_password(Request* request, char* response){
	char answer[512]={0};

	if( request->type == POST ) {
		Log_Message("set passwd");

		char* password[128] ={0};
		if (request_get_post_field(request, "password", password) == 0 ){
			config_set("password", password);
		} else {
			strcpy(password, "FIELD_NOT_FOUND");
		}

		sprintf(answer, "<h2 class=\"sub-header\">Password set</h2><div class=\"table-responsive\">Password set to <code>%s</code></div>", config_get("password"));
	} else {
		strcpy(answer,"<h2 class=\"sub-header\">Set connect password</h2><div class=\"table-responsive\">"
				"<form method=\"POST\">Password: <input name=\"password\" /><br /><input name=\"password2\" /><input type=\"submit\" value=\"Connect\" /></form>"
				"</div>");
	}

	page_index(response, answer);
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
