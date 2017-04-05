#ifndef REQUEST_H
#define REQUEST_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


#define REQUEST_MAX_PATH_LEN 128
#define REQUEST_MAX_PROTO_LEN 16

#define REQUEST_MAX_HEADER_KEY 1024
#define REQUEST_MAX_HEADER_VALUE 1024
#define REQUEST_MAX_HEADER_RAW_KV (REQUEST_MAX_HEADER_KEY+REQUEST_MAX_HEADER_VALUE+1)
#define REQUEST_MAX_HEADER_COUNT 128

#define RESPONSE_MAX_LEN 1024*1

enum REQUEST_TYPE {GET, POST, HEAD, PUT};

typedef struct {
	char key[REQUEST_MAX_HEADER_KEY];
	char value[REQUEST_MAX_HEADER_VALUE];
} Request_Header;

typedef struct {
	uint8_t type;
	int headers_count;
	char path[REQUEST_MAX_PATH_LEN];
	char proto[REQUEST_MAX_PROTO_LEN];
	Request_Header* headers[REQUEST_MAX_HEADER_COUNT];

} Request;

Request* request_init(char* raw_data);
void request_free(Request* request);

#ifdef __cplusplus
}
#endif

#endif
