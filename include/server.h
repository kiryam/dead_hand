#ifndef SERVER_H
#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"


#define MAX_PENDING_DISCONNECTION 10

#define MAX_DATA_SIZE 1024

#define MAX_PACKET_LEN 1024
#define SERVER_PORT 8888


extern int server_status;
extern int clients_count;
extern int open_connections[];

int WIFI_Server_Start(int port);

#ifdef __cplusplus
}
#endif

#endif
