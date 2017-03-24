#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"

int config_set(char* key, char* value);
char* config_get(char* key);

#ifdef __cplusplus
}
#endif

#endif
