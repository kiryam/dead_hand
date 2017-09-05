#ifndef HANDLERS_H
#define HANDLERS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "request.h"

int handle_request(Request* request, uint8_t* response);
int handler_index(Request* request,  char* response);
int handler_style(Request* request,  char* response);
int handler_connect(Request* request,  char* response);
int handler_get_ap_list(Request* request,  char* response);
int handler_ap_connect(Request* request,  char* response);
int handler_manage(Request* request, char* response);
#ifdef TCL_ENABLED
int handler_tcl(Request* request, char* response)
#endif
int handler_forth(Request* request, char* response);
int handler_relay_on(Request* request, char* response);
int handler_relay_off(Request* request, char* response);
int handler_restore(Request* request, char* response);
#ifdef PROTO_LOG
int handler_get_protocol_log(Request* request, char* response)
#endif
int handler_memory(Request* request, char* response);
int handler_favicon(Request* request, char* response);
int handler_404(Request* request, char* response);
int handler_metrics(Request* request, char* response);

#ifdef __cplusplus
}
#endif

#endif
