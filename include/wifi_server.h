#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "wifi.h"

#define REQUEST_MAX_PATH_LEN 128
#define REQUEST_MAX_PROTO_LEN 16

#define REQUEST_MAX_HEADER_KEY 1024
#define REQUEST_MAX_HEADER_VALUE 1024
#define REQUEST_MAX_HEADER_RAW_KV (REQUEST_MAX_HEADER_KEY+REQUEST_MAX_HEADER_VALUE+1)
#define REQUEST_MAX_HEADER_COUNT 128

#define RESPONSE_MAX_LEN 2048

enum REQUEST_TYPE {GET, POST, HEAD, PUT};

typedef struct {
	char key[REQUEST_MAX_HEADER_KEY];
	char value[REQUEST_MAX_HEADER_VALUE];
} Request_Header;

typedef struct {
	uint8_t type;
	char path[REQUEST_MAX_PATH_LEN];
	char proto[REQUEST_MAX_PROTO_LEN];
	Request_Header* headers[REQUEST_MAX_HEADER_COUNT];

} Request;

void WIFI_Server_Init();
void controller_wifi_server(int btn);
void render_wifi_server();

#ifdef __cplusplus
}
#endif

#endif
