#include <stdio.h>
#include <string.h>
#include "request.h"
#include "resources.h"
#include "wifi.h"
#include "display.h"
#include "wifi_status.h"
#include "relay.h"
#include "common.h"
#include "log.h"
#include "favicon.h"

#define UNUSED(x) (void)(x)

#ifdef FORTH_ENABLED
#include "zforth.h"
#endif

#ifdef TCL_ENABLED
#include "tcl.h"
#endif


#ifdef USE_UMM_MALLOC
#include "umm_malloc.h"
#include "umm_malloc_cfg.h"
#endif

int handle_request(Request* request, uint8_t* response){
	uint8_t response_page[RESPONSE_MAX_LEN] = {0};
	size_t response_bytes = 0;

	if (request->path[0] == '/' && request->path[1] == '\0' ) {
		response_bytes = handler_index(request, response_page);
	} else if( strcmp("/manage", request->path) == 0 ){
		response_bytes = handler_manage(request, response_page);
	}
	#ifdef TCL_ENABLED
		else if( strncmp("/tcl", request->path, 4) == 0 ){
		response_bytes = handler_tcl(request, response_page);
	}
	#endif
	#ifdef FORTH_ENABLED
	else if( strncmp("/forth", request->path, 4) == 0 ){
		response_bytes = handler_forth(request, response_page);
	}
	#endif
	else if(strncmp("/memory", request->path, 7) ==0){
		response_bytes = handler_memory(request, response_page);
	} else if(strncmp("/metrics", request->path, 8) ==0){
		response_bytes = handler_metrics(request, response_page);
	} else if(strcmp("/connect", request->path) ==0){
		response_bytes = handler_connect(request, response_page);
	} else if(strcmp("/get_ap_list", request->path) ==0){
		response_bytes = handler_get_ap_list(request, response_page);
	} else if(strcmp("/ap_connect", request->path) == 0){
		response_bytes = handler_ap_connect(request, response_page);
	} else if(strcmp("/relay_on", request->path) == 0){
		response_bytes = handler_relay_on(request, response_page);
	} else if(strcmp("/relay_off", request->path) == 0){
		response_bytes = handler_relay_off(request, response_page);
	} else if(strcmp("/restore", request->path) == 0){
		response_bytes = handler_restore(request, response_page);
	} else if(strncmp("/favicon.ico", request->path, 11) == 0) {
		response_bytes = handler_favicon(request, response_page);
	} else if(strcmp("/style.css", request->path) == 0) {
		response_bytes = handler_style(request, response_page);
	} else if(strcmp("/reset", request->path) == 0) {
		NVIC_SystemReset();
	} else {
		response_bytes = handler_404(request, response_page);
	}


	strcat(response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: ");
	char tmp[8] = {0};
	itoa(response_bytes, tmp, 10);
	strcat(response, tmp);
	strcat(response, "\n\r\n");
	size_t header_len=0;
	header_len = strlen(response);
	memcpy(&response[header_len], response_page, response_bytes);

	return header_len+response_bytes;// sprintf(response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\n\r\n%s", response_bytes, response_page);
}

int page_index(char* page_index, char* content){
	return sprintf(page_index, static_template_page, static_style, static_javascript, content);
}

int handler_index(Request* request,  char* response) {
	UNUSED(request);
	Log_Message_FAST("index");
	return page_index(response, "<h2 class=\"sub-header\">Dashboard</h2><div class=\"table-responsive\"></div>");
}

int handler_style(Request* request,  char* response) {
	UNUSED(request);
	strcpy(response, static_style);
	return strlen(static_style);

}

int handler_connect(Request* request,  char* response) {
	UNUSED(request);
	strcpy(response, static_page_connect);
	return strlen(static_page_connect);
}

int handler_get_ap_list(Request* request,  char* response) {
	UNUSED(request);
	WIFI_List_Result* result = malloc_c(sizeof(WIFI_List_Result));
	if (result == NULL){
		Log_Message("Out of memory");
		return 0;
	}

	int point_list_err = WIFI_Retreive_List(result);

	if( point_list_err == 0 ){
		for(int i=0; i<result->found;i++){
			WIFI_Point* point = result->points[i];
			char line[256]={0};
			sprintf(line, "%s\n", point->name);
			strcat(response, line);
			Log_Message_FAST(point->name);
		}
	}

	WIFI_List_Result_Free(result);
	return strlen(response);
}

int handler_ap_connect(Request* request,  char* response) {
	if( request->type == POST ) {
		char* ssid[REQUEST_DATA_VALUE_LEN] ={0};
		char* password[REQUEST_DATA_VALUE_LEN] ={0};
		if (request_get_post_field(request, "ssid", ssid) != 0 ){
			strcpy(response, "FIELD_SSID_GET_ERROR");
			return strlen(response);
		}
		if (request_get_post_field(request, "password", password) != 0 ){
			strcpy(response, "FIELD_PASSWORD_GET_ERROR");
			return strlen(response);
		}

		if ( WIFI_Connect(ssid, password) == 0 ){
			Log_Message("Connected");
			strcpy(response, "Connected");
			Display_Set_Active_Controller(controller_wifi_status);
			Display_Set_Active_Render(render_wifi_status);
			WIFI_Status_Init();
		}else{
			Log_Message("Failed to connect");
			strcpy(response, "Failed to connect");
		}
	}else{
		strcpy(response, "Method not allowed");
	}

	return strlen(response);
}

int handler_manage(Request* request, char* response) {
	UNUSED(request);

	char manage_response[4096] = {0};
	if( RELAY_STATE == RELAY_ON ){
		sprintf(manage_response, static_page_manage,"<a href=\"/relay_on\"><button type=\"button\" class=\"btn btn-default\">On</button></a><a href=\"/relay_off\"><button type=\"button\" class=\"btn btn-success\">Off</button></a>");
	}else{
		sprintf(manage_response, static_page_manage,"<a href=\"/relay_on\"><button type=\"button\" class=\"btn btn-success\">On</button></a><a href=\"/relay_off\"><button type=\"button\" class=\"btn btn-default\">Off</button></a>");
	}
	return page_index(response, manage_response);
}

#ifdef TCL_ENABLED
int handler_tcl(Request* request, char* response){
	if( request->type == POST ) {
		char s[REQUEST_DATA_VALUE_LEN] = {0};
		if (request_get_post_field(request, "code", s) != 0){
			sprintf(response, "CODE_FIELD_NOT_FOUND");
			return;
		}

		struct tcl tcl;
		tcl_init(&tcl);

		if (tcl_eval(&tcl, s, strlen(s)) != FERROR) {
		    sprintf(response, "%.*s\n", tcl_length(tcl.result), tcl_string(tcl.result));
		    tcl_destroy(&tcl);
		    return;
		}
		tcl_destroy(&tcl);

		sprintf(response, "ERROR");
		return;
	}
	return page_index(response, static_page_tcl);
}
#endif

#ifdef FORTH_ENABLED
int handler_forth(Request* request, char* response){
	if( request->type == POST ) {
		char s[REQUEST_DATA_VALUE_LEN] = {0};
		if (request_get_post_field(request, "code", s) != 0){
			sprintf(response, "CODE_FIELD_NOT_FOUND");
			return strlen(response);
		}

		zf_result r = zf_eval(s);
		if(r != ZF_OK) {
			switch (r) {
			case ZF_ABORT_INTERNAL_ERROR:
				sprintf(response, "ZF_ABORT_INTERNAL_ERROR");
				break;

			case ZF_ABORT_OUTSIDE_MEM:
				sprintf(response, "ZF_ABORT_OUTSIDE_MEM");
				break;

			case ZF_ABORT_DSTACK_UNDERRUN:
				sprintf(response, "ZF_ABORT_DSTACK_UNDERRUN");
				break;

			case ZF_ABORT_DSTACK_OVERRUN:
				sprintf(response, "ZF_ABORT_DSTACK_OVERRUN");
				break;

			case ZF_ABORT_RSTACK_UNDERRUN:
				sprintf(response, "ZF_ABORT_RSTACK_UNDERRUN");
				break;

			case ZF_ABORT_RSTACK_OVERRUN:
					sprintf(response, "ZF_ABORT_RSTACK_OVERRUN");
					break;

			case ZF_ABORT_NOT_A_WORD:
					sprintf(response, "ZF_ABORT_NOT_A_WORD");
					break;

			case ZF_ABORT_COMPILE_ONLY_WORD:
					sprintf(response, "ZF_ABORT_COMPILE_ONLY_WORD");
					break;

			case ZF_ABORT_INVALID_SIZE:
					sprintf(response, "ZF_ABORT_INVALID_SIZE");
				break;
			}

			return strlen(response);
		}

		forth_answer[forth_answer_cursor] = '\0';
		strncpy(response, forth_answer, FORTH_ANSWER_MAX_LENGTH);
		strcat(response, " ok");

		forth_answer[0] = '\0';
		forth_answer_cursor = 0;
		return strlen(response);
	}
	return page_index(response, static_page_forth);
}

#endif



int handler_relay_on(Request* request, char* response) {
	UNUSED(request);
	RELAY_Off();
	return handler_manage(request, response);
}

int handler_relay_off(Request* request, char* response) {
	UNUSED(request);
	RELAY_On();
	return handler_manage(request, response);
}


int handler_restore(Request* request, char* response) {
	UNUSED(request);
	strcpy(response, "ESP8266 will restores now");
	if (WIFI_Restore() == 0){
		Log_Message("ESP8266 successfully restored");
		NVIC_SystemReset();
	}
	return page_index(response, "");
}

#ifdef PROTO_LOG
int handler_get_protocol_log(Request* request, char* response){
	UNUSED(request);
	Log_Message("Get protocol log");
	strncpy(response, get_protocol_log(), RESPONSE_MAX_LEN);
	return strlen(response);
}
#endif


int handler_memory(Request* request, char* response) {
	UNUSED(request);
#ifdef USE_UMM_MALLOC
	umm_info(NULL, 0);
	return sprintf(response, "TotalBlocks: %d<br />UsedBlocks: %d<br />FreeBlocks: %d<br />",  (size_t)ummHeapInfo.totalBlocks,  (size_t)ummHeapInfo.usedBlocks, (size_t)ummHeapInfo.freeBlocks);
#endif

#ifdef USE_TLSF
	Log_Message("Memory");
	return sprintf(response, "Allocated memory: %d of %d", get_memory_allocated_total(), get_memory_max());
#endif
}

int handler_favicon(Request* request, char* response) {
	UNUSED(request);
	Log_Message_FAST("Favicon");
	memcpy(response, static_favicon, static_favicon_length);
	return static_favicon_length;
}

int handler_404(Request* request, char* response) {
	UNUSED(request);
	Log_Message_FAST("404");
	strcpy(response, "404 Page not found");
	return strlen(response);
}

int handler_metrics(Request* request, char* response) {
	UNUSED(request);
	Log_Message_FAST("metric");
	return sprintf(response, "memory_used %d\nmemory_max_used %d\n", get_memory_allocated_total(), get_memory_max());
}
