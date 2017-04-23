#ifndef HANDLERS_H
#define HANDLERS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "request.h"

int handle_request(Request* request, char* response);

#ifdef __cplusplus
}
#endif

#endif
