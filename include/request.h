#ifndef REQUEST_H
#define REQUEST_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


#define REQUEST_MAX_PATH_LEN 128
#define REQUEST_MAX_PROTO_LEN 16

#define REQUEST_MAX_HEADER_KEY 128
#define REQUEST_MAX_HEADER_VALUE 128
#define REQUEST_MAX_HEADER_RAW_KV (REQUEST_MAX_HEADER_KEY+REQUEST_MAX_HEADER_VALUE+1)
#define REQUEST_MAX_HEADER_COUNT 128




#define RESPONSE_MAX_LEN 1024*6


#define REQUEST_DATA_KEY_LEN 40
#define REQUEST_DATA_VALUE_LEN 1024
#define REQUEST_MAX_DATA_COUNT 64
#define REQUEST_MAX_DATA_RAW_KV (REQUEST_DATA_KEY_LEN+REQUEST_DATA_VALUE_LEN+1)


enum REQUEST_TYPE {GET, POST, HEAD, PUT};

typedef struct {
	char key[REQUEST_MAX_HEADER_KEY];
	char value[REQUEST_MAX_HEADER_VALUE];
} Request_Header;


typedef struct {
	char key[REQUEST_DATA_KEY_LEN];
	char value[REQUEST_DATA_VALUE_LEN];
} Request_Data;


typedef struct {
	uint8_t type;
	int headers_count;
	int data_count;
	char path[REQUEST_MAX_PATH_LEN];
	char proto[REQUEST_MAX_PROTO_LEN];
	Request_Header* headers[REQUEST_MAX_HEADER_COUNT];
	Request_Data * data[REQUEST_MAX_DATA_COUNT];

	char tmp_field[REQUEST_MAX_HEADER_KEY]; // need for stream parsing
} Request;

int request_parse(Request* request, char* data, unsigned int length);
void request_free(Request* request);

#ifdef __cplusplus
}
#endif

#endif
