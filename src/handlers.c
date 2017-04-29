#include <stdio.h>
#include <string.h>
#include "request.h"
#include <resources.h>
#include "wifi.h"
#include "display.h"
#include "wifi_status.h"
#include "relay.h"
#include "common.h"

#ifdef USE_UMM_MALLOC
#include "umm_malloc.h"
#include "umm_malloc_cfg.h"
#endif

int handle_request(Request* request, char* response){
	uint8_t response_page[RESPONSE_MAX_LEN] = {0};

	if (request->path[0] == '/' && request->path[1] == '\0' ) {
		handler_index(request, response_page);
	} else if( strcmp("/manage", request->path) == 0 ){
		handler_manage(request, response_page);
	} else if( strncmp("/tcl", request->path, 4) == 0 ){
		handler_tcl(request, response_page);
	}else if(strncmp("/memory", request->path, 7) ==0){
		handler_memory(request, response_page);
	} else if(strncmp("/metrics", request->path, 8) ==0){
		handler_metrics(request, response_page);
	} else if(strcmp("/connect", request->path) ==0){
		handler_connect(request, response_page);
	} else if(strcmp("/get_ap_list", request->path) ==0){
		handler_get_ap_list(request, response_page);
	} else if(strcmp("/ap_connect", request->path) == 0){
		handler_ap_connect(request, response_page);
	} else if(strcmp("/relay_on", request->path) == 0){
		handler_relay_on(request, response_page);
	} else if(strcmp("/relay_off", request->path) == 0){
		handler_relay_off(request, response_page);
	} else if(strcmp("/restore", request->path) == 0){
		handler_restore(request, response_page);
	} else if(strncmp("/favicon.ico", request->path, 11) == 0) {
		handler_favicon(request, response_page);
	} else if(strcmp("/style.css", request->path) == 0) {
		handler_style(request, response_page);
	} else if(strcmp("/reset", request->path) == 0) {
		NVIC_SystemReset();
	} else {
		handler_404(request, response_page);
	}

	return sprintf(response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\n\r\n%s", strlen(response_page), response_page);
}

int page_index(char* page_index, char* content){
	sprintf(page_index, static_template_page, static_style, static_javascript, content);
	return 0;
}

void handler_index(Request* request,  char* response) {
	Log_Message("index");
	page_index(response, "<h2 class=\"sub-header\">Dashboard</h2><div class=\"table-responsive\"></div>");
}

void handler_style(Request* request,  char* response){
	strcpy(response, static_style);
}

void handler_connect(Request* request,  char* response){
	strcpy(response, static_page_connect);
}

void handler_get_ap_list(Request* request,  char* response){
	WIFI_List_Result* result = malloc_c(sizeof(WIFI_List_Result));
	if (result == NULL){
		Log_Message("Out of memory");
		return;
	}

	int point_list_err = WIFI_Retreive_List(result);

	if( point_list_err == 0 ){
		for(int i=0; i<result->found;i++){
			WIFI_Point* point = result->points[i];
			char line[256]={0};
			sprintf(line, "%s\n", point->name);
			strcat(response, line);
			Log_Message(point->name);
		}
	}

	WIFI_List_Result_Free(result);
}

void handler_ap_connect(Request* request,  char* response){
	if( request->type == POST ) {
		char* ssid[REQUEST_DATA_VALUE_LEN] ={0};
		char* password[REQUEST_DATA_VALUE_LEN] ={0};
		if (request_get_post_field(request, "ssid", ssid) != 0 ){
			strcpy(response, "FIELD_SSID_GET_ERROR");
			return;
		}
		if (request_get_post_field(request, "password", password) != 0 ){
			strcpy(response, "FIELD_PASSWORD_GET_ERROR");
			return;
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
}

void handler_manage(Request* request, char* response){
	char manage_response[4096] = {0};
	if( RELAY_STATE == RELAY_ON ){
		sprintf(manage_response, static_page_manage,"<a href=\"/relay_on\"><button type=\"button\" class=\"btn btn-default\">On</button></a><a href=\"/relay_off\"><button type=\"button\" class=\"btn btn-success\">Off</button></a>");
	}else{
		sprintf(manage_response, static_page_manage,"<a href=\"/relay_on\"><button type=\"button\" class=\"btn btn-success\">On</button></a><a href=\"/relay_off\"><button type=\"button\" class=\"btn btn-default\">Off</button></a>");
	}
	page_index(response, manage_response);
}

void handler_tcl(Request* request, char* response){
	if( request->type == POST ) {
		char s[4096] = {0};
		if (request_get_post_field(request, "code", s) != 0){
			sprintf(response, "CODE_FIELD_NOT_FOUND");
			return;
		}

		/*
		struct tcl tcl;
		tcl_init(&tcl);

		if (tcl_eval(&tcl, s, strlen(s)) != FERROR) {
		    sprintf(response, "%.*s\n", tcl_length(tcl.result), tcl_string(tcl.result));
		    tcl_destroy(&tcl);
		    return;
		}
		tcl_destroy(&tcl);
		*/
		sprintf(response, "ERROR");
		return;
	}
	page_index(response, static_page_tcl);
}



void handler_relay_on(Request* request, char* response){
	RELAY_Off();
	handler_manage(request, response);
}

void handler_relay_off(Request* request, char* response){
	RELAY_On();
	handler_manage(request, response);
}


void handler_restore(Request* request, char* response){
	strcpy(response, "ESP8266 will restores now");
	if (WIFI_Restore() == 0){
		Log_Message("ESP8266 successfully restored");
		NVIC_SystemReset();
	}
	page_index(response, "");
}

#ifdef PROTO_LOG
void handler_get_protocol_log(Request* request, char* response){
	Log_Message("Get protocol log");
	strncpy(response, get_protocol_log(), RESPONSE_MAX_LEN);
}
#endif


void handler_memory(Request* request, char* response){
#ifdef USE_UMM_MALLOC
	umm_info(NULL, 0);
	sprintf(response, "TotalBlocks: %d<br />UsedBlocks: %d<br />FreeBlocks: %d<br />",  (size_t)ummHeapInfo.totalBlocks,  (size_t)ummHeapInfo.usedBlocks, (size_t)ummHeapInfo.freeBlocks);
#endif

#ifdef USE_TLSF
	Log_Message("Memory");
	sprintf(response, "Allocated memory: %d of %d", get_memory_allocated_total(), get_memory_max());
#endif
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
