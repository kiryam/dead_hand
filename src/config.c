#include "config.h"
#include <stdio.h>

typedef struct {
	uint8_t is_initialized;
	char password[40];
} Config_Storage;


#define CONFIG_STORAGE ( Config_Storage *)(BKPSRAM_BASE)

int config_set(char* key, char* value){
	if( strcmp(key, "password") == 0 ){
		Config_Storage* storage = CONFIG_STORAGE;
		storage->is_initialized = 1;
		strcpy(storage->password, value);
		return 0;
	}
	return 1;
}

char* config_get(char* key){
	if (strcmp(key, "password") == 0){
		Config_Storage* storage = CONFIG_STORAGE;
		if ( storage->is_initialized != 1 ){
			return "default_password";
		}
		return storage->password;
	}
	return NULL;
}
